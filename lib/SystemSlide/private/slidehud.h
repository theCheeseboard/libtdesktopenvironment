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
#ifndef SLIDEHUD_H
#define SLIDEHUD_H

#include <QWidget>

namespace Ui {
    class SlideHud;
}

class SlideHud : public QWidget {
        Q_OBJECT

    public:
        explicit SlideHud(QWidget* parent = nullptr);
        ~SlideHud();

        void setAction(QString action, QString description);
        void setActionIcon(QIcon icon);

    private:
        Ui::SlideHud* ui;

        void paintEvent(QPaintEvent* event);
};

#endif // SLIDEHUD_H
