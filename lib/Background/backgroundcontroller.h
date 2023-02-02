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
#ifndef BACKGROUNDCONTROLLER_H
#define BACKGROUNDCONTROLLER_H

#include <QCoroTask>
#include <QObject>
#include <tpromise.h>

class QNetworkReply;

struct BackgroundControllerPrivate;
class BackgroundController : public QObject {
        Q_OBJECT
    public:
        enum BackgroundType {
            Desktop,
            LockScreen
        };

        enum StretchType {
            StretchFit = 0,
            ZoomCrop = 1,
            Center = 2,
            Tile = 3,
            ZoomFit = 4
        };

        struct BackgroundData {
                QPixmap px;
                bool extendedInfoAvailable = false;

                QString name;
                QString location;
                QString author;
        };

        explicit BackgroundController(BackgroundType type, QObject* parent = nullptr);
        ~BackgroundController();

        QCoro::Task<BackgroundData> getCurrentBackground(QSize screenSize);
        QCoro::Task<BackgroundData> getBackground(QString backgroundName, QSize screenSize);
        QStringList availableBackgrounds();

        void setBackground(QString backgroundName, BackgroundType type);
        QString currentBackgroundName(BackgroundType type);

        void setStretchType(StretchType type);
        StretchType stretchType();

        void setShouldShowCommunityLabels(bool shouldShowCommunityLabels);
        bool shouldShowCommunityLabels();

    signals:
        void currentBackgroundChanged(BackgroundType type);
        void stretchTypeChanged(StretchType type);
        void shouldShowCommunityLabelsChanged(bool shouldShowCommunityLabels);
        void newCommunityBackgroundsAvailable();
        void currentCommuntyBackgroundAvailable(QPixmap background);
        void availableWallpapersChanged(int newWallpapers);

    private:
        BackgroundControllerPrivate* d;

        void timerEvent(QTimerEvent* event);

        QNetworkReply* get(QString path);
        QCoro::Task<> getNewCommunityBackground();
        QCoro::Task<BackgroundData> getCurrentCommunityBackground();
        uint communityBackgroundPeriod();
        QCoro::Task<QStringList> searchWallpapers(QString searchPath);
};

class BackgroundException : public QException {
    public:
        void raise() const override { throw *this; }
        BackgroundException* clone() const override { return new BackgroundException(*this); }
};

#endif // BACKGROUNDCONTROLLER_H
