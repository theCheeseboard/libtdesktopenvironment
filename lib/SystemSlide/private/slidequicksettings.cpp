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
#include "slidequicksettings.h"
#include "ui_slidequicksettings.h"

#include "../../theShellIntegration/quietmodemanager.h"
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <texception.h>
#include <tswitch.h>

struct SlideQuickSettingsPrivate {
        QuietModeManager* quietMode;
};

SlideQuickSettings::SlideQuickSettings(QuietModeManager* quietMode, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SlideQuickSettings) {
    ui->setupUi(this);

    d = new SlideQuickSettingsPrivate();
    d->quietMode = quietMode;

    this->addToggle(tr("Flight Mode"));

    connect(d->quietMode, &QuietModeManager::quietModeChanged, this, &SlideQuickSettings::quietModeStateChanged);
    quietModeStateChanged();
}

SlideQuickSettings::~SlideQuickSettings() {
    delete ui;
    delete d;
}

void SlideQuickSettings::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
}

void SlideQuickSettings::mousePressEvent(QMouseEvent* event) {
    event->accept();
}

void SlideQuickSettings::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();
}

tSwitch* SlideQuickSettings::addToggle(QString title) {
    QWidget* w = new QWidget(this);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    w->setLayout(layout);

    QLabel* l = new QLabel(this);
    l->setText(title);
    layout->addWidget(l);

    tSwitch* s = new tSwitch(this);
    layout->addWidget(s);

    ui->quickSettingsContainer->addWidget(w);
    return s;
}

QCoro::Task<> SlideQuickSettings::quietModeStateChanged() {
    try {
        auto qm = co_await d->quietMode->quietMode();
        switch (qm) {
            case QuietModeManager::None:
                ui->quietModeSound->setChecked(true);
                break;
            case QuietModeManager::Critical:
                ui->quietModeCriticalOnly->setChecked(true);
                break;
            case QuietModeManager::Notifications:
                ui->quietModeNoNotifications->setChecked(true);
                break;
            case QuietModeManager::Mute:
                ui->quietModeMute->setChecked(true);
                break;
        }

        ui->quietModePane->setVisible(true);
    } catch (tDBusException ex) {
        ui->quietModePane->setVisible(false);
    }
}

void SlideQuickSettings::on_quietModeSound_toggled(bool checked) {
    if (checked) d->quietMode->setQuietMode(QuietModeManager::None);
}

void SlideQuickSettings::on_quietModeCriticalOnly_toggled(bool checked) {
    if (checked) d->quietMode->setQuietMode(QuietModeManager::Critical);
}

void SlideQuickSettings::on_quietModeNoNotifications_toggled(bool checked) {
    if (checked) d->quietMode->setQuietMode(QuietModeManager::Notifications);
}

void SlideQuickSettings::on_quietModeMute_toggled(bool checked) {
    if (checked) d->quietMode->setQuietMode(QuietModeManager::Mute);
}
