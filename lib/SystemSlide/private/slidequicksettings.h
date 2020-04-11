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
#ifndef SLIDEQUICKSETTINGS_H
#define SLIDEQUICKSETTINGS_H

#include <QWidget>

namespace Ui {
    class SlideQuickSettings;
}

class tSwitch;
class QuietModeManager;
struct SlideQuickSettingsPrivate;
class SlideQuickSettings : public QWidget {
        Q_OBJECT

    public:
        explicit SlideQuickSettings(QuietModeManager* quietMode, QWidget* parent = nullptr);
        ~SlideQuickSettings();

    private slots:
        void on_quietModeSound_toggled(bool checked);

        void on_quietModeCriticalOnly_toggled(bool checked);

        void on_quietModeNoNotifications_toggled(bool checked);

        void on_quietModeMute_toggled(bool checked);

    private:
        Ui::SlideQuickSettings* ui;
        SlideQuickSettingsPrivate* d;

        void paintEvent(QPaintEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);

        tSwitch* addToggle(QString title);

        void quietModeStateChanged();
};

#endif // SLIDEQUICKSETTINGS_H
