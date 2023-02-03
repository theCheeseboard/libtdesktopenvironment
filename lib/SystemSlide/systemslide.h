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
#ifndef SYSTEMSLIDE_H
#define SYSTEMSLIDE_H

#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class SystemSlide;
}

struct SystemSlidePrivate;
class SystemSlide : public QWidget {
        Q_OBJECT

    public:
        enum BackgroundMode {
            NoBackground,
            DesktopBackground,
            LockScreenBackground,
            DefaultBackground,
            CommunityBackground
        };

        explicit SystemSlide(QWidget* parent);
        ~SystemSlide();

        void setAction(QString action, QString description);
        void setActionIcon(QIcon icon);
        void setDragResultWidget(QWidget* widget);
        void setSideWidget(QWidget* sideWidget);

        void setBackgroundMode(BackgroundMode mode);

        void activate();
        void deactivate();
        bool isActive();

        void setDeactivateSpeedThreshold(int speedThreshold);
        void setDeactivateOnClick(bool deactivateOnClick);

        void mprisPlayPause();
        void mprisBack();
        void mprisNext();

    signals:
        void deactivated();

    private:
        Ui::SystemSlide* ui;
        SystemSlidePrivate* d;

        void upowerStateChanged();
        QCoro::Task<> quietModeStateChanged();
        QCoro::Task<> backgroundChanged();

        void pulseAudioDataAvailable(const float* data, int length);

        void showQuickSettings();
        void hideQuickSettings();

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);

        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
};

#endif // SYSTEMSLIDE_H
