/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "backgroundcontroller.h"

#include <QCoroFuture>
#include <QCoroNetworkReply>
#include <QCoroSignal>
#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QStandardPaths>
#include <QSvgRenderer>

struct BackgroundControllerPrivate {
        QNetworkAccessManager mgr;
        QSettings* settings;
        QFileSystemWatcher* settingsWatcher;

        BackgroundController::BackgroundType type;
        bool retrievingImages = false;
        int downloadCount = 0;

        int timerId;
        uint lastPeriod = 0;

        QStringList additionalWallpapers;
};

BackgroundController::BackgroundController(BackgroundType type, QObject* parent) :
    QObject(parent) {
    Q_INIT_RESOURCE(libtdesktopenvironment_resources);

    d = new BackgroundControllerPrivate();
    d->settings = new QSettings("theSuite", "theShell");
    d->type = type;

    d->timerId = this->startTimer(60000, Qt::VeryCoarseTimer);

    // Find backgrounds from /usr/share/wallpapers and /usr/share/backgrounds
    searchWallpapers("/usr/share/wallpapers").then([this](QStringList wallpapers) {
        d->additionalWallpapers.append(wallpapers);
        emit availableWallpapersChanged(wallpapers.count());
    });
    searchWallpapers("/usr/share/backgrounds").then([this](QStringList wallpapers) {
        d->additionalWallpapers.append(wallpapers);
        emit availableWallpapersChanged(wallpapers.count());
    });
}

BackgroundController::~BackgroundController() {
    delete d;
}

QCoro::Task<BackgroundController::BackgroundData> BackgroundController::getCurrentBackground(QSize screenSize) {
    co_return co_await this->getBackground(currentBackgroundName(d->type), screenSize);
}

QCoro::Task<BackgroundController::BackgroundData> BackgroundController::getBackground(QString backgroundName, QSize screenSize) {
    BackgroundData data;
    data.px = QPixmap(screenSize);

    auto drawBackground = [this, screenSize](QPixmap pixmap) {
        QPixmap newPixmap(screenSize);

        QPainter painter(&newPixmap);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Draw newPixmap
        switch (d->settings->value("desktop/stretchStyle", 0).toInt()) {
            case 0: // Stretch
                painter.drawPixmap(0, 0, newPixmap.width(), newPixmap.height(), pixmap);
                break;
            case 1:
                { // Zoom and Crop
                    QRect rect;
                    rect.setSize(pixmap.size().scaled(newPixmap.width(), newPixmap.height(), Qt::KeepAspectRatioByExpanding));
                    rect.moveLeft(newPixmap.width() / 2 - rect.width() / 2);
                    rect.moveTop(newPixmap.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, pixmap);
                    break;
                }
            case 2:
                { // Center
                    QRect rect;
                    rect.setSize(pixmap.size());
                    rect.moveLeft(newPixmap.width() / 2 - rect.width() / 2);
                    rect.moveTop(newPixmap.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, pixmap);
                    break;
                }
            case 3: // Tile
                painter.drawTiledPixmap(0, 0, newPixmap.width(), newPixmap.height(), pixmap);
                break;
            case 4:
                { // Zoom and Fit
                    QRect rect;
                    rect.setSize(pixmap.size().scaled(newPixmap.width(), newPixmap.height(), Qt::KeepAspectRatio));
                    rect.moveLeft(newPixmap.width() / 2 - rect.width() / 2);
                    rect.moveTop(newPixmap.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, pixmap);
                    break;
                }
        }

        return newPixmap;
    };

    if (backgroundName.startsWith("inbuilt:")) { // Inbuilt background
        QSvgRenderer renderer(QStringLiteral(":/libtdesktopenvironment/backgrounds/%1.svg").arg(backgroundName.mid(backgroundName.indexOf(':') + 1)));
        if (!renderer.isValid()) throw BackgroundException();
        QPainter painter(&data.px);
        renderer.render(&painter, data.px.rect());

        co_return data;
    } else if (backgroundName.startsWith("community")) {
        QDir::home().mkpath(".theshell/backgrounds");
        bool metadataExists = QFile(QDir::homePath() + "/.theshell/backgrounds.conf").exists();
        bool expired = d->settings->value("desktop/fetched").toDateTime().secsTo(QDateTime::currentDateTimeUtc()) > 604800 /* 1 week */;

        if (!metadataExists || expired) {
            // Get a new community background, choose a random one and show it
            co_await this->getNewCommunityBackground();
        }

        data = co_await this->getCurrentCommunityBackground();
        data.px = drawBackground(data.px);
        co_return data;
    } else {
        QPixmap image;
        if (!image.load(backgroundName)) throw BackgroundException();

        data.px = drawBackground(image);
        co_return data;
    }
}

QStringList BackgroundController::availableBackgrounds() {
    QStringList backgrounds = {
        "community",
        "inbuilt:cactus",
        "inbuilt:triangles",
        "inbuilt:ribbon"};

    backgrounds.append(d->additionalWallpapers);
    backgrounds.append({"inbuilt:arrows",
        "inbuilt:beach",
        "inbuilt:leftwaves",
        "inbuilt:nav",
        "inbuilt:shatter",
        "inbuilt:slice",
        "inbuilt:triplecircle",
        "inbuilt:waves",
        currentBackgroundName(BackgroundType::Desktop),
        currentBackgroundName(BackgroundType::LockScreen),
        "custom"});
    backgrounds.removeDuplicates();
    return backgrounds;
}

void BackgroundController::setBackground(QString backgroundName, BackgroundController::BackgroundType type) {
    QString key;
    if (type == Desktop) {
        key = "desktop/background";
    } else if (type == LockScreen) {
        key = "lockScreen/background";
    }
    d->settings->setValue(key, backgroundName);
    emit currentBackgroundChanged(type);
}

QString BackgroundController::currentBackgroundName(BackgroundType type) {
    QString key;
    if (type == Desktop) {
        key = "desktop/background";
    } else if (type == LockScreen) {
        key = "lockScreen/background";
    }
    return d->settings->value(key, "inbuilt:cactus").toString();
}

void BackgroundController::setStretchType(BackgroundController::StretchType type) {
    d->settings->setValue("desktop/stretchStyle", static_cast<int>(type));
    emit stretchTypeChanged(type);
}

BackgroundController::StretchType BackgroundController::stretchType() {
    return static_cast<StretchType>(d->settings->value("desktop/stretchStyle", StretchFit).toInt());
}

void BackgroundController::setShouldShowCommunityLabels(bool shouldShowCommunityLabels) {
    d->settings->setValue("desktop/showLabels", shouldShowCommunityLabels);
    emit shouldShowCommunityLabelsChanged(shouldShowCommunityLabels);
}

bool BackgroundController::shouldShowCommunityLabels() {
    return d->settings->value("desktop/showLabels", true).toBool();
}

void BackgroundController::timerEvent(QTimerEvent* event) {
    // Check to see if the community background has changed
    if (event->timerId() == d->timerId) {
        uint currentPeriod = this->communityBackgroundPeriod();
        if (d->lastPeriod != 0 && d->lastPeriod != currentPeriod) {
            if (currentBackgroundName(BackgroundController::Desktop) == "community") emit currentBackgroundChanged(BackgroundController::Desktop);
            if (currentBackgroundName(BackgroundController::LockScreen) == "community") emit currentBackgroundChanged(BackgroundController::LockScreen);
        }
        d->lastPeriod = currentPeriod;
    }
}

QNetworkReply* BackgroundController::get(QString path) {
    QUrl url;
    url.setScheme("https");
    url.setHost("vicr123.com");
    url.setPath(path);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("theShell"));
    QNetworkReply* reply = d->mgr.get(req);
    return reply;
}

QCoro::Task<> BackgroundController::getNewCommunityBackground() {
    if (d->retrievingImages) {
        co_await qCoro(this, &BackgroundController::newCommunityBackgroundsAvailable);
        co_return;
    }

    try {
        d->retrievingImages = true;
        auto reply = get("/theshell/backgrounds/backgrounds.json");
        co_await reply;

        auto data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isArray()) {
            // Error
            reply->deleteLater();
            throw BackgroundException();
        } else {
            QJsonArray arr = doc.array();
            arr.removeFirst();

            QStringList downloadImages;

            for (int i = 0; i < 10; i++) {
                int index = QRandomGenerator::system()->bounded(arr.count());
                QString url = arr.at(index).toString();
                downloadImages.append(url);
                arr.removeAt(index);

                if (arr.count() == 0) i = 10;
            }

            // Keep track of time when these images were retrieved
            d->settings->setValue("desktop/fetched", QDateTime::currentDateTimeUtc());

            // Delete list of known images
            QFile(QDir::homePath() + "/.theshell/backgrounds.conf").remove();
            QDir(QDir::homePath() + "/.theshell/backgrounds/").removeRecursively();
            QDir::home().mkpath(".theshell/backgrounds/");

            d->downloadCount = 0;
            for (QString url : downloadImages) {
                auto reply = co_await get(url);
                QByteArray data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();

                    QString fileName = obj.value("filename").toString();
                    QString dirName = fileName.left(fileName.indexOf("."));

                    QDir::home().mkpath(".theshell/backgrounds/" + dirName);
                    QFile metadataFile(QDir::homePath() + "/.theshell/backgrounds/" + dirName + "/metadata.json");
                    metadataFile.open(QFile::WriteOnly);
                    metadataFile.write(data);
                    metadataFile.close();

                    QFile backgroundListConf(QDir::homePath() + "/.theshell/backgrounds.conf");
                    backgroundListConf.open(QFile::Append);
                    backgroundListConf.write(dirName.append("\n").toUtf8());
                    backgroundListConf.close();
                }
                reply->deleteLater();
                d->downloadCount++;

                if (d->downloadCount == downloadImages.count()) {
                    d->downloadCount = 0;
                    emit newCommunityBackgroundsAvailable();
                    co_return;
                }
            }

            reply->deleteLater();
        }
        d->retrievingImages = false;
    } catch (QException ex) {
        d->retrievingImages = true;
    }
}

QCoro::Task<BackgroundController::BackgroundData> BackgroundController::getCurrentCommunityBackground() {
    QFile backgroundListConf(QDir::homePath() + "/.theshell/backgrounds.conf");
    backgroundListConf.open(QFile::ReadOnly);
    QStringList allBackgrounds = QString(backgroundListConf.readAll()).split("\n");
    backgroundListConf.close();

    allBackgrounds.removeAll("");

    // TODO: Fix algorithm
    QRandomGenerator generator(this->communityBackgroundPeriod());

    QString background = allBackgrounds.at(generator.bounded(allBackgrounds.count()));

    QFile metadataFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/metadata.json");
    metadataFile.open(QFile::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll());
    metadataFile.close();

    if (!doc.isObject()) {
        throw BackgroundException();
    }

    if (!QFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg").exists()) {
        QString fileName = doc.object().value("filename").toString();
        QString dirName = fileName.left(fileName.indexOf("."));

        auto reply = co_await get(QStringLiteral("/theshell/backgrounds/%1/%2").arg(dirName, fileName));
        if (reply->error() != QNetworkReply::NoError) {
            // Error retrieving image
            throw BackgroundException();
        }

        QByteArray data = reply->readAll();
        QFile imageFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg");
        imageFile.open(QFile::WriteOnly);
        imageFile.write(data);
        imageFile.close();
        reply->deleteLater();
    }

    QFile imageFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg");
    QJsonObject metadata = doc.object();

    BackgroundData data;
    data.px.load(imageFile.fileName());

    data.extendedInfoAvailable = true;
    data.name = metadata.value("name").toString();
    data.location = metadata.value("location").toString();
    data.author = metadata.value("author").toString();

    co_return data;
}

uint BackgroundController::communityBackgroundPeriod() {
    return static_cast<uint>(QDateTime::currentSecsSinceEpoch() / (30 * 60));
}

QCoro::Task<QStringList> BackgroundController::searchWallpapers(QString searchPath) {
    co_return co_await QtConcurrent::run([this](QString searchPath) {
        QStringList wallpapers;
        struct WallpaperInformation {
                int w, h;
                QString suffix;

                bool operator<(WallpaperInformation other) {
                    return (w * h) < (other.w * other.h);
                }
        };

        QMap<QString, WallpaperInformation> largestImages;
        QDirIterator iterator(searchPath, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            QFileInfo fi = iterator.fileInfo();
            if (QStringList({"png", "jpg", "jpeg"}).contains(fi.suffix())) {
                if (fi.filePath().contains("contents") && QRegularExpression("^(\\d+x\\d+)|(screenshot)$").match(fi.baseName()).hasMatch()) {
                    if (fi.baseName() == "screenshot") continue; // Ignore

                    // Find only the largest image in this set
                    WallpaperInformation info;
                    info.w = fi.fileName().split("x").first().toInt();
                    info.h = fi.fileName().split("x").at(1).split(".").first().toInt();
                    info.suffix = fi.suffix();

                    WallpaperInformation oldInfo = largestImages.value(fi.path(), {0, 0, ""});
                    if (oldInfo < info) {
                        largestImages.insert(fi.path(), info);
                    }
                } else {
                    wallpapers.append(fi.filePath());
                }
            }
        }

        for (QString path : largestImages.keys()) {
            WallpaperInformation info = largestImages.value(path);
            wallpapers.append(QStringLiteral("%1/%2x%3.%4").arg(path).arg(info.w).arg(info.h).arg(info.suffix));
        }

        return wallpapers;
    },
        searchPath);
}
