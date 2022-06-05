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
#include "slidehud.h"
#include "ui_slidehud.h"

#include <QIcon>
#include <QPainter>
#include <libcontemporary_global.h>

SlideHud::SlideHud(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SlideHud) {
    ui->setupUi(this);
}

SlideHud::~SlideHud() {
    delete ui;
}

void SlideHud::setAction(QString action, QString description) {
    ui->actionLabel->setText(action);
    ui->descriptionLabel->setText(description);
}

void SlideHud::setActionIcon(QIcon icon) {
    ui->iconLabel->setPixmap(icon.pixmap(SC_DPI_T(QSize(16, 16), QSize)));
}

void SlideHud::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, 0, this->width(), 0);
}
