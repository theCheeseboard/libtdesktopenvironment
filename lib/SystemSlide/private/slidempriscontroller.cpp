/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#include "slidempriscontroller.h"
#include "ui_slidempriscontroller.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QPainter>
#include <the-libs_global.h>
#include <QMenu>
#include "../../mpris/mprisengine.h"

struct SlideMprisControllerPrivate {
    QMenu* mprisSelection;
    QActionGroup* menuActionsGroup;
    QMap<QString, QAction*> menuActions;
    MprisPlayerPtr player;
    QObject* mprisContextObject = nullptr;
    QPalette defaultPal;
    QNetworkAccessManager mgr;

    void addServer(QString service, MprisPlayerPtr player);
    void removeServer(QString service);
    void setServer(MprisPlayerPtr player);

    SlideMprisController* parent;
    Ui::SlideMprisController* ui;
};

SlideMprisController::SlideMprisController(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SlideMprisController) {
    ui->setupUi(this);

    d = new SlideMprisControllerPrivate();
    d->parent = this;
    d->ui = ui;
    d->menuActionsGroup = new QActionGroup(this);
    d->defaultPal = this->palette();

    d->mprisSelection = new QMenu(this);
    d->mprisSelection->addSection(tr("Select Media Player"));
    ui->mprisSelection->setMenu(d->mprisSelection);
    ui->mprisSelection->setVisible(false);

    for (MprisPlayerPtr player : MprisEngine::players()) {
        d->addServer(player->service(), player);
    }
    connect(MprisEngine::instance(), &MprisEngine::newPlayer, this, std::bind(&SlideMprisControllerPrivate::addServer, d, std::placeholders::_1, std::placeholders::_2));
    connect(MprisEngine::instance(), &MprisEngine::playerGone, this, std::bind(&SlideMprisControllerPrivate::removeServer, d, std::placeholders::_1));

    if (MprisEngine::players().count() == 0) {
        d->setServer(nullptr);
    } else {
        d->setServer(MprisEngine::players().first());
    }
}

SlideMprisController::~SlideMprisController() {
    delete ui;
    delete d;
}

void SlideMprisController::playPause() {
    if (d->player) d->player->playPause();
}

void SlideMprisController::next() {
    if (d->player) d->player->next();
}

void SlideMprisController::back() {
    if (d->player) d->player->previous();
}

void SlideMprisController::on_mprisPlay_clicked() {
    playPause();
}

void SlideMprisController::on_mprisNext_clicked() {
    next();
}

void SlideMprisController::on_mprisBack_clicked() {
    back();
}

void SlideMprisControllerPrivate::addServer(QString service, MprisPlayerPtr player) {
    QAction* serverAction = new QAction();

    serverAction->setText(player->identity());
    serverAction->setCheckable(true);

    mprisSelection->addAction(serverAction);
    menuActions.insert(service, serverAction);
    menuActionsGroup->addAction(serverAction);

    QObject::connect(player.data(), &MprisPlayer::identityChanged, serverAction, [ = ] {
        serverAction->setText(player->identity());
    });
    QObject::connect(serverAction, &QAction::triggered, [ = ] {
        this->setServer(player);
    });

    if (MprisEngine::players().count() > 1) ui->mprisSelection->setVisible(true);
}

void SlideMprisControllerPrivate::removeServer(QString service) {
    QAction* serverAction = menuActions.value(service);
    mprisSelection->removeAction(serverAction);
    menuActionsGroup->removeAction(serverAction);
    menuActions.remove(service);
    serverAction->deleteLater();

    if (this->player->service() == service) {
        if (MprisEngine::players().count() == 0) {
            this->setServer(nullptr);
        } else {
            this->setServer(MprisEngine::players().first());
        }
    }

    if (MprisEngine::players().count() <= 1) ui->mprisSelection->setVisible(false);
}

void SlideMprisControllerPrivate::setServer(MprisPlayerPtr player) {
    this->player = player;

    //Set up the context object
    if (mprisContextObject != nullptr) mprisContextObject->deleteLater();
    mprisContextObject = new QObject();

    if (player == nullptr) {
        parent->setVisible(false);
    } else {
        this->menuActions.value(player->service())->setChecked(true);
        parent->setVisible(true);

        auto setMetadataFunction = [ = ] {
            QString statusString;
            QString title = player->metadata().value("xesam:title").toString();
            if (title == "") {
                title = player->identity();
            } else {
                QStringList parts;
                parts.append(player->metadata().value("xesam:artist").toStringList().join(", "));
                parts.append(player->metadata().value("xesam:album").toString());

                statusString = parts.join(" · ");
            }

            ui->mprisTitle->setText(title);
            ui->mprisMetadata->setText(statusString);

            MetadataMap map = player->metadata();
            QString albumArt = player->metadata().value("mpris:artUrl").toString();
            ui->mprisIcon->setPixmap(QIcon::fromTheme("audio").pixmap(SC_DPI_T(QSize(48, 48), QSize)));
            parent->setPalette(defaultPal);
            if (albumArt != "") {
                QNetworkRequest req((QUrl(albumArt)));
                QNetworkReply* reply = mgr.get(req);
                QObject::connect(reply, &QNetworkReply::finished, [ = ] {
                    if (reply->error() == QNetworkReply::NoError) {
                        QImage image = QImage::fromData(reply->readAll());
                        if (!image.isNull()) {
                            image = image.scaled(SC_DPI(48), SC_DPI(48), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                            qulonglong red = 0, green = 0, blue = 0;

                            QPalette pal = defaultPal;
                            int totalPixels = 0;
                            for (int i = 0; i < image.width(); i++) {
                                for (int j = 0; j < image.height(); j++) {
                                    QColor c = image.pixelColor(i, j);
                                    if (c.alpha() != 0) {
                                        red += c.red();
                                        green += c.green();
                                        blue += c.blue();
                                        totalPixels++;
                                    }
                                }
                            }

                            QColor c;
                            int averageCol = (pal.color(QPalette::Window).red() + pal.color(QPalette::Window).green() + pal.color(QPalette::Window).blue()) / 3;

                            if (totalPixels == 0) {
                                if (averageCol < 127) {
                                    c = pal.color(QPalette::Window).darker(200);
                                } else {
                                    c = pal.color(QPalette::Window).lighter(200);
                                }
                            } else {
                                c = QColor(red / totalPixels, green / totalPixels, blue / totalPixels);

                                if (averageCol < 127) {
                                    c = c.darker(200);
                                } else {
                                    c = c.lighter(200);
                                }
                            }

                            pal.setColor(QPalette::Window, c);
                            parent->setPalette(pal);

                            QImage rounded(SC_DPI_T(QSize(48, 48), QSize), QImage::Format_ARGB32);
                            rounded.fill(Qt::transparent);
                            QPainter p(&rounded);
                            p.setRenderHint(QPainter::Antialiasing);
                            p.setBrush(QBrush(image));
                            p.setPen(Qt::transparent);
                            p.drawRoundedRect(0, 0, SC_DPI(48), SC_DPI(48), 40, 40, Qt::RelativeSize);

                            ui->mprisIcon->setPixmap(QPixmap::fromImage(rounded));
                        }
                    }
                    reply->deleteLater();
                });
            }

        };
        setMetadataFunction();

        if (player->playbackStatus() == MprisPlayer::Playing) {
            ui->mprisPlay->setIcon(QIcon::fromTheme("media-playback-pause"));
        } else {
            ui->mprisPlay->setIcon(QIcon::fromTheme("media-playback-start"));
        }
        QObject::connect(player.data(), &MprisPlayer::metadataChanged, mprisContextObject, setMetadataFunction);
        QObject::connect(player.data(), &MprisPlayer::playbackStatusChanged, mprisContextObject, [ = ] {
            if (player->playbackStatus() == MprisPlayer::Playing) {
                ui->mprisPlay->setIcon(QIcon::fromTheme("media-playback-pause"));
            } else {
                ui->mprisPlay->setIcon(QIcon::fromTheme("media-playback-start"));
            }
        });
    }
}
