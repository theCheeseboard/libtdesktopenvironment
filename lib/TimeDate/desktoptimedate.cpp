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
#include "desktoptimedate.h"

#include <QLocale>
#include <QDateTime>
#include <QSettings>
#include <QTimer>
#include <QLabel>

struct DesktopTimeDatePrivate {
    QMap<QLabel*, DesktopTimeDate::StringType> updates;
    QTimer* updateTimer = nullptr;
};

DesktopTimeDatePrivate* DesktopTimeDate::d = new DesktopTimeDatePrivate();

QString DesktopTimeDate::timeString(QDateTime d, DesktopTimeDate::StringType type) {
    QSettings settings("theSuite", "theShell");
    bool use24Hour = settings.value("time/use24hour", true).toBool();

    QLocale loc;
    QString amPm;
    if (!use24Hour) {
        if (d.time().hour() > 12) {
            amPm = loc.pmText();
            d = d.addSecs(-43200);
        } else if (d.time().hour() == 12) {
            amPm = loc.pmText();
        } else if (d.time().hour() == 0) {
            amPm = loc.amText();
            d = d.addSecs(43200);
        } else {
            amPm = loc.amText();
        }
    }

    switch (type) {
        case FullTime:
            if (use24Hour) {
                return d.time().toString("HH:mm:ss");
            } else {
                return (d.time().toString("hh:mm:ss") + amPm);
            }
        case Time:
            return d.time().toString("HH:mm:ss");
        case AmPm:
            return amPm.toLower();
        case StandardDate:
            return loc.toString(d, tr("ddd dd MMM yyyy"));
    }
}

QString DesktopTimeDate::timeString(DesktopTimeDate::StringType type) {
    return timeString(QDateTime::currentDateTime(), type);
}

void DesktopTimeDate::makeTimeLabel(QLabel* label, DesktopTimeDate::StringType type) {
    if (!d->updateTimer) {
        d->updateTimer = new QTimer();
        d->updateTimer->setInterval(1000);
        QObject::connect(d->updateTimer, &QTimer::timeout, [ = ] {
            for (auto i = d->updates.begin(); i != d->updates.end(); i++) {
                i.key()->setText(timeString(i.value()));
            }
        });
        d->updateTimer->start();
    }

    //Decorate the label
    QFont fnt = label->font();
    if (type == AmPm) {
        fnt.setCapitalization(QFont::SmallCaps);
    }
    label->setFont(fnt);

    d->updates.insert(label, type);
}
