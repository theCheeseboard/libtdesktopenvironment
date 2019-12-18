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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSvgRenderer>
#include <QPainter>
#include <QStandardPaths>

struct BackgroundControllerPrivate {
    QNetworkAccessManager mgr;
    QSettings* settings;

    BackgroundController::BackgroundType type;
    bool retrievingImages = false;
    int downloadCount = 0;

    int timerId;
    uint lastPeriod = 0;
};

BackgroundController::BackgroundController(BackgroundType type, QObject *parent) : QObject(parent)
{
    Q_INIT_RESOURCE(libtdesktopenvironment_resources);

    d = new BackgroundControllerPrivate();
    d->settings = new QSettings("theSuite", "theShell");
    d->type = type;

    d->timerId = this->startTimer(60000, Qt::VeryCoarseTimer);
}

BackgroundController::~BackgroundController()
{
    delete d;
}

tPromise<BackgroundController::BackgroundData>* BackgroundController::getCurrentBackground(QSize screenSize)
{
    return this->getBackground(currentBackgroundName(d->type), screenSize);
}

tPromise<BackgroundController::BackgroundData>*BackgroundController::getBackground(QString backgroundName, QSize screenSize)
{
    return tPromise<BackgroundData>::runOnSameThread([=](tPromiseFunctions<BackgroundData>::SuccessFunction res, tPromiseFunctions<BackgroundData>::FailureFunction rej) {
        BackgroundData data;
        data.px = QPixmap(screenSize);

        if (backgroundName.startsWith("inbuilt:")) { //Inbuilt background
            QSvgRenderer renderer(QStringLiteral(":/libtdesktopenvironment/backgrounds/%1.svg").arg(backgroundName.mid(backgroundName.indexOf(':') + 1)));
            if (!renderer.isValid()) {
                rej("Unavailable Background");
                return;
            }
            QPainter painter(&data.px);
            renderer.render(&painter, data.px.rect());

            res(data);
        } else if (backgroundName.startsWith("community")) {
            QDir::home().mkpath(".theshell/backgrounds");
            bool metadataExists = QFile(QDir::homePath() + "/.theshell/backgrounds.conf").exists();
            bool expired = d->settings->value("desktop/fetched").toDateTime().secsTo(QDateTime::currentDateTimeUtc()) > 604800 /* 1 week */;

            auto chooseBackground = [=] {
                this->getCurrentCommunityBackground()->then([=](BackgroundData data) {
                    QPixmap background(screenSize);
                    QPainter painter(&background);

                    //Draw background
                    switch (d->settings->value("desktop/stretchStyle", 0).toInt()) {
                        case 0: //Stretch
                            painter.drawPixmap(0, 0, background.width(), background.height(), data.px);
                            break;
                        case 1: { //Zoom and Crop
                            QRect rect;
                            rect.setSize(data.px.size().scaled(background.width(), background.height(), Qt::KeepAspectRatioByExpanding));
                            rect.moveLeft(background.width() / 2 - rect.width() / 2);
                            rect.moveTop(background.height() / 2 - rect.height() / 2);
                            painter.drawPixmap(rect, data.px);
                            break;
                        }
                        case 2: { //Center
                            QRect rect;
                            rect.setSize(data.px.size());
                            rect.moveLeft(background.width() / 2 - rect.width() / 2);
                            rect.moveTop(background.height() / 2 - rect.height() / 2);
                            painter.drawPixmap(rect, data.px);
                            break;
                        }
                        case 3: //Tile
                            painter.drawTiledPixmap(0, 0, background.width(), background.height(), data.px);
                            break;
                        case 4: { //Zoom and Fit
                            QRect rect;
                            rect.setSize(data.px.size().scaled(background.width(), background.height(), Qt::KeepAspectRatio));
                            rect.moveLeft(background.width() / 2 - rect.width() / 2);
                            rect.moveTop(background.height() / 2 - rect.height() / 2);
                            painter.drawPixmap(rect, data.px);
                            break;
                        }
                    }

                    data.px = background;
                    res(data);
                })->error([=](QString error) {
                    rej(error);
                });
            };

            if (metadataExists && !expired) {
                //Choose a random community background and show it
                chooseBackground();
            } else {
                //Get a new community background, choose a random one and show it
                this->getNewCommunityBackground()->then([=] {
                    chooseBackground();
                })->error([=](QString error) {
                    rej(error);
                });
            }
        } else {
            QPixmap image;
            if (!image.load(backgroundName)) {
                rej("Invalid File");
                return;
            }

            QPainter painter(&data.px);

            //Clear background
            painter.setBrush(QColor(0, 0, 0));
            painter.setPen(Qt::transparent);
            painter.drawRect(0, 0, data.px.width(), data.px.height());

            //Draw background
            switch (d->settings->value("desktop/stretchStyle", 0).toInt()) {
                case 0: //Stretch
                    painter.drawPixmap(0, 0, data.px.width(), data.px.height(), image);
                    break;
                case 1: { //Zoom and Crop
                    QRect rect;
                    rect.setSize(image.size().scaled(data.px.width(), data.px.height(), Qt::KeepAspectRatioByExpanding));
                    rect.moveLeft(data.px.width() / 2 - rect.width() / 2);
                    rect.moveTop(data.px.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, image);
                    break;
                }
                case 2: { //Center
                    QRect rect;
                    rect.setSize(image.size());
                    rect.moveLeft(data.px.width() / 2 - rect.width() / 2);
                    rect.moveTop(data.px.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, image);
                    break;
                }
                case 3: //Tile
                    painter.drawTiledPixmap(0, 0, data.px.width(), data.px.height(), image);
                    break;
                case 4: { //Zoom and Fit
                    QRect rect;
                    rect.setSize(image.size().scaled(data.px.width(), data.px.height(), Qt::KeepAspectRatio));
                    rect.moveLeft(data.px.width() / 2 - rect.width() / 2);
                    rect.moveTop(data.px.height() / 2 - rect.height() / 2);
                    painter.drawPixmap(rect, image);
                    break;
                }
            }

            res(data);
        }
    });
}

QStringList BackgroundController::availableBackgrounds()
{
    QStringList backgrounds = {
        "community",
        "inbuilt:triangles",
        "inbuilt:ribbon",
        "inbuilt:arrows",
        "inbuilt:beach",
        "inbuilt:leftwaves",
        "inbuilt:nav",
        "inbuilt:shatter",
        "inbuilt:slice",
        "inbuilt:triplecircle",
        "inbuilt:waves",
        currentBackgroundName(BackgroundType::Desktop),
        currentBackgroundName(BackgroundType::LockScreen),
        "custom"
    };
    backgrounds.removeDuplicates();
    return backgrounds;
}

void BackgroundController::setBackground(QString backgroundName, BackgroundController::BackgroundType type)
{
    QString key;
    if (type == Desktop) {
        key = "desktop/background";
    } else if (type == LockScreen) {
        key = "lockScreen/background";
    }
    d->settings->setValue(key, backgroundName);
    emit currentBackgroundChanged(type);
}

QString BackgroundController::currentBackgroundName(BackgroundType type)
{
    QString key;
    if (type == Desktop) {
        key = "desktop/background";
    } else if (type == LockScreen) {
        key = "lockScreen/background";
    }
    return d->settings->value(key, "inbuilt:triangles").toString();
}

void BackgroundController::setStretchType(BackgroundController::StretchType type)
{
    d->settings->setValue("desktop/stretchStyle", static_cast<int>(type));
    emit stretchTypeChanged(type);
}

BackgroundController::StretchType BackgroundController::stretchType()
{
    return static_cast<StretchType>(d->settings->value("desktop/stretchStyle", StretchFit).toInt());
}

void BackgroundController::setShouldShowCommunityLabels(bool shouldShowCommunityLabels)
{
    d->settings->setValue("desktop/showLabels", shouldShowCommunityLabels);
    emit shouldShowCommunityLabelsChanged(shouldShowCommunityLabels);
}

bool BackgroundController::shouldShowCommunityLabels()
{
    return d->settings->value("desktop/showLabels", true).toBool();
}

void BackgroundController::timerEvent(QTimerEvent*event)
{
    //Check to see if the community background has changed
    if (event->timerId() == d->timerId) {
        uint currentPeriod = this->communityBackgroundPeriod();
        if (d->lastPeriod != 0 && d->lastPeriod != currentPeriod) {
            if (currentBackgroundName(BackgroundController::Desktop) == "community") emit currentBackgroundChanged(BackgroundController::Desktop);
            if (currentBackgroundName(BackgroundController::LockScreen) == "community") emit currentBackgroundChanged(BackgroundController::LockScreen);
        }
        d->lastPeriod = currentPeriod;
    }
}

tPromise<QNetworkReply*>*BackgroundController::get(QString path)
{
    return tPromise<QNetworkReply*>::runOnSameThread([=](tPromiseFunctions<QNetworkReply*>::SuccessFunction res, tPromiseFunctions<QNetworkReply*>::FailureFunction rej) {
        QUrl url;
        url.setScheme("https");
        url.setHost("vicr123.com");
        url.setPath(path);

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QString("theShell"));
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* reply = d->mgr.get(req);

        connect(reply, &QNetworkReply::finished, this, [=] {
            res(reply);
        });
    });
}

tPromise<void>* BackgroundController::getNewCommunityBackground() {
    return tPromise<void>::runOnSameThread([=](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        if (d->retrievingImages) {
            QMetaObject::Connection* c = new QMetaObject::Connection();
            *c = connect(this, &BackgroundController::newCommunityBackgroundsAvailable, this, [=] {
                disconnect(*c);
                delete c;

                res();
            });
            return;
        }

        get("/theshell/backgrounds/backgrounds.json")->then([=](QNetworkReply* reply) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);

            if (!doc.isArray()) {
                //Error
                rej("Invalid backgrond data received");
                reply->deleteLater();
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

                //Keep track of time when these images were retrieved
                d->settings->setValue("desktop/fetched", QDateTime::currentDateTimeUtc());

                //Delete list of known images
                QFile(QDir::homePath() + "/.theshell/backgrounds.conf").remove();
                QDir(QDir::homePath() + "/.theshell/backgrounds/").removeRecursively();
                QDir::home().mkpath(".theshell/backgrounds/");

                d->downloadCount = 0;
                for (QString url : downloadImages) {
                    get(url)->then([=](QNetworkReply* reply) {
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
                            res();
                        }
                    });
                }

                reply->deleteLater();
            }
        });
    });


}

tPromise<BackgroundController::BackgroundData>* BackgroundController::getCurrentCommunityBackground()
{
    return tPromise<BackgroundData>::runOnSameThread([=](tPromiseFunctions<BackgroundData>::SuccessFunction res, tPromiseFunctions<BackgroundData>::FailureFunction rej) {
        QFile backgroundListConf(QDir::homePath() + "/.theshell/backgrounds.conf");
        backgroundListConf.open(QFile::ReadOnly);
        QStringList allBackgrounds = QString(backgroundListConf.readAll()).split("\n");
        backgroundListConf.close();

        allBackgrounds.removeAll("");

        //TODO: Fix algorithm
        QRandomGenerator generator(this->communityBackgroundPeriod());

        QString background = allBackgrounds.at(generator.bounded(allBackgrounds.count()));

        QFile metadataFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/metadata.json");
        metadataFile.open(QFile::ReadOnly);
        QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll());
        metadataFile.close();

        if (!doc.isObject()) {
            rej("Metadata File Corrupt");
            return;
        }

        auto readBackground = [=] {
            QFile imageFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg");
            QJsonObject metadata = doc.object();

            BackgroundData data;
            data.px.load(imageFile.fileName());

            data.extendedInfoAvailable = true;
            data.name = metadata.value("name").toString();
            data.location = metadata.value("location").toString();
            data.author = metadata.value("author").toString();

            res(data);
        };

        if (QFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg").exists()) {
            readBackground();
        } else {
            QString fileName = doc.object().value("filename").toString();
            QString dirName = fileName.left(fileName.indexOf("."));

            get(QStringLiteral("/theshell/backgrounds/%1/%2").arg(dirName, fileName))->then([=](QNetworkReply* reply) {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QFile imageFile(QDir::homePath() + "/.theshell/backgrounds/" + background + "/" + background + ".jpeg");
                    imageFile.open(QFile::WriteOnly);
                    imageFile.write(data);
                    imageFile.close();
                    reply->deleteLater();

                    //Set the background
                    readBackground();
                } else {
                    //Error retrieving image
                    rej("Background Not Available");
                }
            })->error([=](QString error) {
                rej(error);
            });
        }
    });
}

uint BackgroundController::communityBackgroundPeriod()
{
    return static_cast<uint>(QDateTime::currentSecsSinceEpoch() / (30 * 60));
}
