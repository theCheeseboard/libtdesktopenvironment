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
#include "desktopupowerdevice.h"

#include <QApplication>
#include <QDBusInterface>
#include <QIcon>
#include <QPainter>
#include <QPalette>
#include <QTime>
#include <libcontemporary_global.h>

struct DesktopUPowerDevicePrivate {
        QDBusInterface* interface;

        bool settingUp = true;

        bool notification25 = false;
        bool notification10 = false;
        bool notification5 = false;
        DesktopUPowerDevice::DeviceState oldState = DesktopUPowerDevice::UnknownState;
};

DesktopUPowerDevice::DesktopUPowerDevice(QDBusObjectPath path, QObject* parent) :
    QObject(parent) {
    d = new DesktopUPowerDevicePrivate();

    d->interface = new QDBusInterface("org.freedesktop.UPower", path.path(), "org.freedesktop.UPower.Device", QDBusConnection::systemBus());
    QDBusConnection::systemBus().connect("org.freedesktop.UPower", path.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SIGNAL(propertiesUpdated()));

    connect(this, &DesktopUPowerDevice::propertiesUpdated, this, [=] {
        if (this->type() == Battery) {
            DeviceState state = this->state();
            if (d->oldState != Charging && state == Charging) {
                d->notification5 = false;
                d->notification10 = false;
                d->notification25 = false;

                if (!d->settingUp) emit chargingNotification();
                d->oldState = state;
                d->settingUp = false;
                return;
            } else if ((d->oldState == Charging || d->oldState == FullyCharged) && state != Charging && state != FullyCharged) {
                d->notification5 = false;
                d->notification10 = false;
                d->notification25 = false;

                if (!d->settingUp) emit dischargingNotification();
                d->oldState = state;
                d->settingUp = false;
                return;
            } else if (d->oldState != FullyCharged && state == FullyCharged) {
                d->notification5 = false;
                d->notification10 = false;
                d->notification25 = false;

                if (!d->settingUp) emit fullNotification();
                d->oldState = state;
                d->settingUp = false;
                return;
            } else if (state == Charging) {
                // Return early so that the low battery notification is not displayed
                d->settingUp = false;
                return;
            }
            d->oldState = state;
        }

        int percentage = this->percentage();
        if (percentage <= 5 && !d->notification5) {
            d->notification25 = true;
            d->notification10 = true;
            d->notification5 = true;

            if (!d->settingUp) emit lowBatteryNotification(tr("About %1% remaining").arg(percentage));
        } else if (percentage <= 10 && !d->notification10) {
            d->notification25 = true;
            d->notification10 = true;

            if (!d->settingUp) emit lowBatteryNotification(tr("About %1% remaining").arg(percentage));
        } else if (percentage <= 25 && !d->notification25) {
            d->notification25 = true;

            if (!d->settingUp) emit lowBatteryNotification(tr("About %1% remaining").arg(percentage));
        }

        d->settingUp = false;
    });
}

DesktopUPowerDevice::~DesktopUPowerDevice() {
    d->interface->deleteLater();
    delete d;
}

DesktopUPowerDevice::DeviceType DesktopUPowerDevice::type() {
    return static_cast<DeviceType>(d->interface->property("Type").toUInt());
}

QString DesktopUPowerDevice::typeString() {
    const char* types[] = {
        QT_TR_NOOP("Unknown"),
        QT_TR_NOOP("AC Power"),
        QT_TR_NOOP("Battery"),
        QT_TR_NOOP("Uninterruptable Power Supply"),
        QT_TR_NOOP("Display"),
        QT_TR_NOOP("Mouse"),
        QT_TR_NOOP("Keyboard"),
        QT_TR_NOOP("PDA"),
        QT_TR_NOOP("Phone")};

    return tr(types[this->type()]);
}

DesktopUPowerDevice::DeviceState DesktopUPowerDevice::state() {
    return static_cast<DeviceState>(d->interface->property("State").toUInt());
}

QString DesktopUPowerDevice::stateString() {
    const char* states[] = {
        QT_TR_NOOP("Unknown"),
        QT_TR_NOOP("Charging"),
        QT_TR_NOOP("Discharging"),
        QT_TR_NOOP("Empty"),
        QT_TR_NOOP("Fully Charged"),
        QT_TR_NOOP("Pending Charge"),
        QT_TR_NOOP("Pending Discharge")};

    DeviceState state = this->state();
    if (state == Charging) {
        QTime timeToFull = QTime::fromMSecsSinceStartOfDay(static_cast<int>(this->timeToFull()) * 1000);
        if (timeToFull.msecsSinceStartOfDay() < 1000) {
            return tr("Charging");
        } else {
            return tr("Charging · %1 until full").arg(timeToFull.toString("hh:mm"));
        }
    } else if (state == Discharging) {
        QTime timeToEmpty = QTime::fromMSecsSinceStartOfDay(static_cast<int>(this->timeToEmpty()) * 1000);
        if (timeToEmpty.msecsSinceStartOfDay() < 1000) {
            return tr("Discharging");
        } else {
            return tr("Discharging · %1 until empty").arg(timeToEmpty.toString("hh:mm"));
        }
    } else {
        return tr(states[this->state()]);
    }
}

int DesktopUPowerDevice::percentage() {
    return static_cast<int>(d->interface->property("Percentage").toDouble());
}

bool DesktopUPowerDevice::online() {
    return d->interface->property("Online").toBool();
}

QString DesktopUPowerDevice::iconName() {
    switch (this->type()) {
        case DesktopUPowerDevice::UnknownType:
            break;
        case DesktopUPowerDevice::LinePower:
            return "ac-adapter";
        case DesktopUPowerDevice::Battery:
            {
                if (this->state() == DesktopUPowerDevice::Charging) {
                    if (this->percentage() < 10) {
                        return "battery-charging-empty";
                    } else if (this->percentage() < 30) {
                        return "battery-charging-020";
                    } else if (this->percentage() < 50) {
                        return "battery-charging-040";
                    } else if (this->percentage() < 70) {
                        return "battery-charging-060";
                    } else if (this->percentage() < 90) {
                        return "battery-charging-080";
                    } else {
                        return "battery-charging-100";
                    }
                } else {
                    if (this->percentage() < 10) {
                        return "battery-empty";
                    } else if (this->percentage() < 30) {
                        return "battery-020";
                    } else if (this->percentage() < 50) {
                        return "battery-040";
                    } else if (this->percentage() < 70) {
                        return "battery-060";
                    } else if (this->percentage() < 90) {
                        return "battery-080";
                    } else {
                        return "battery-100";
                    }
                }
            }
        case DesktopUPowerDevice::Ups:
            return "ups";
        case DesktopUPowerDevice::Monitor:
            return "video-display";
        case DesktopUPowerDevice::Mouse:
            return "input-mouse";
        case DesktopUPowerDevice::Keyboard:
            return "input-keyboard";
        case DesktopUPowerDevice::Pda:
        case DesktopUPowerDevice::Phone:
            return "phone";
    }
    return "";
}

QIcon DesktopUPowerDevice::icon() {
    if (QIcon::themeName() != "contemporary" || this->type() != DesktopUPowerDevice::Battery) return QIcon::fromTheme(iconName());

    QIcon icon;

    QList<QSize> sizes = {
        QSize(16, 16),
        QSize(24, 24),
        QSize(32, 32),
        QSize(48, 48),
        QSize(64, 64)};

    QPalette pal = QApplication::palette();
    QColor col = pal.color(QPalette::WindowText);
    QColor transparentCol = col;
    transparentCol.setAlpha(127);

    QList<DesktopUPowerDevice::DeviceState> chargingStates = {
        DesktopUPowerDevice::Charging,
        DesktopUPowerDevice::FullyCharged};

    for (QSize size : sizes) {
        QImage image(size, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setWindow(QRect(-1, -1, 2, 2));

        QRectF bounds(-0.9, -0.9, 1.8, 1.8);
        painter.setPen(Qt::transparent);
        painter.setBrush(transparentCol);
        painter.drawEllipse(bounds);

        painter.setPen(QPen(col, 0.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(bounds, 1440, -this->percentage() * 57.6);

        if (chargingStates.contains(this->state())) {
            QPolygonF zap;
            zap.append(QPointF(0, -0.6));
            zap.append(QPointF(-0.3, 0.1));
            zap.append(QPointF(0, 0.1));
            zap.append(QPointF(0, 0.6));
            zap.append(QPointF(0.3, -0.1));
            zap.append(QPointF(0, -0.1));

            painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
            painter.setPen(QPen(Qt::transparent, 0));
            painter.setBrush(Qt::transparent);
            painter.drawPolygon(zap);
        }

        painter.end();

        icon.addPixmap(QPixmap::fromImage(image));
    }

    return icon;
}

qint64 DesktopUPowerDevice::timeToEmpty() {
    return d->interface->property("TimeToEmpty").toLongLong();
}

qint64 DesktopUPowerDevice::timeToFull() {
    return d->interface->property("TimeToFull").toLongLong();
}
