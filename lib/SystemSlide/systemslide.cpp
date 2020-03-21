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
#include "systemslide.h"
#include "ui_systemslide.h"

#include <QIcon>
#include <QPainter>
#include <QPointer>
#include <tpropertyanimation.h>
#include <QTimer>

#include "private/slidehud.h"

#include "../TimeDate/desktoptimedate.h"
#include "../UPower/desktopupower.h"
#include "../Background/backgroundcontroller.h"

struct SystemSlidePrivate {
    SlideHud* hud;
    QWidget* coverWidget;
    QPointer<QWidget> dragResult;

    SystemSlide::BackgroundMode bgMode;
    DesktopUPower* upower;

    static BackgroundController* bg;
    bool retrieving = false;
    bool retrieveAgain = false;
    BackgroundController::BackgroundData background;
};

BackgroundController* SystemSlidePrivate::bg = nullptr;

SystemSlide::SystemSlide(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SystemSlide) {
    ui->setupUi(this);

    d = new SystemSlidePrivate();

    d->upower = new DesktopUPower(this);

    d->hud = new SlideHud(this);
    d->hud->setFixedHeight(d->hud->sizeHint().height());
    d->hud->show();
    this->setContentsMargins(0, 0, 0, d->hud->sizeHint().height());

    d->coverWidget = new QWidget(this);
    d->coverWidget->setAutoFillBackground(true);
    d->coverWidget->move(0, this->height());
    d->coverWidget->show();

    this->resize(this->parentWidget()->size());
    this->show();
    this->raise();

    d->coverWidget->installEventFilter(this);
    d->hud->installEventFilter(this);
    parent->installEventFilter(this);

    DesktopTimeDate::makeTimeLabel(ui->clockLabel, DesktopTimeDate::Time);
    DesktopTimeDate::makeTimeLabel(ui->clockAmPmLabel, DesktopTimeDate::AmPm);
    DesktopTimeDate::makeTimeLabel(ui->dateLabel, DesktopTimeDate::StandardDate);

    connect(d->upower, &DesktopUPower::overallStateChanged, this, &SystemSlide::upowerStateChanged);
    upowerStateChanged();
}

SystemSlide::~SystemSlide() {
    delete ui;
    delete d;
}

void SystemSlide::setAction(QString action, QString description) {
    d->hud->setAction(action, description);
}

void SystemSlide::setActionIcon(QIcon icon) {
    d->hud->setActionIcon(icon);
}

void SystemSlide::setDragResultWidget(QWidget* widget) {
    d->dragResult = widget;
}

void SystemSlide::setBackgroundMode(SystemSlide::BackgroundMode mode) {
    d->bgMode = mode;
    ui->backgroundInformation->setVisible(false);

    if (!d->bg && (mode == DesktopBackground || mode == LockScreenBackground || mode == CommunityBackground)) {
        BackgroundController::BackgroundType type = BackgroundController::Desktop;
        if (mode == LockScreenBackground) type = BackgroundController::LockScreen;
        d->bg = new BackgroundController(type);
    }

    connect(d->bg, &BackgroundController::currentBackgroundChanged, this, &SystemSlide::backgroundChanged);
    connect(d->bg, &BackgroundController::shouldShowCommunityLabelsChanged, this, [ = ] {
        if (d->bg->currentBackgroundName(BackgroundController::Desktop) == "community") this->backgroundChanged();
    });
    connect(d->bg, &BackgroundController::stretchTypeChanged, this, &SystemSlide::backgroundChanged);

    this->backgroundChanged();
}

void SystemSlide::activate() {
    emit activated();

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(d->hud->y());
    anim->setEndValue(-d->hud->height());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        d->hud->move(0, value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    connect(anim, &tVariantAnimation::finished, this, &SystemSlide::hide);
}

void SystemSlide::deactivate() {
    this->show();

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(d->hud->y());
    anim->setEndValue(this->height() - d->hud->height());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        d->hud->move(0, value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
}

void SystemSlide::upowerStateChanged() {
    if (d->upower->shouldShowOverallState()) {
        ui->upowerIcon->setPixmap(d->upower->overallStateIcon().pixmap(SC_DPI_T(QSize(16, 16), QSize)));
        ui->upowerLabel->setText(d->upower->overallStateDescription());
        ui->upowerPane->setVisible(true);
    } else {
        ui->upowerPane->setVisible(false);
    }
}

void SystemSlide::backgroundChanged() {
    if (d->bgMode != DesktopBackground && d->bgMode != LockScreenBackground && d->bgMode != CommunityBackground) return;

    if (d->retrieving) {
        d->retrieveAgain = true;
        return;
    }

    d->retrieving = true;
//    ui->stackedWidget->setCurrentWidget(ui->loadingBackgroundPage);

    d->bg->getCurrentBackground(this->size())->then([ = ](BackgroundController::BackgroundData data) {
        d->background = data;

        if (d->background.extendedInfoAvailable) {
            if (data.name.isEmpty()) {
                ui->backgroundTitleLabel->setVisible(false);
            } else {
                ui->backgroundTitleLabel->setText(data.name);
                ui->backgroundTitleLabel->setVisible(true);
            }


            if (!data.location.isEmpty()) {
//                painter.setFont(QFont(this->font().family(), 10));
//                QIcon locationIcon = QIcon::fromTheme("gps");
//                int height = painter.fontMetrics().height();
//                int width = painter.fontMetrics().horizontalAdvance(data.location) + height;

//                painter.drawPixmap(currentX, baselineY - height, locationIcon.pixmap(SC_DPI_T(QSize(16, 16), QSize)));
//                painter.drawText(currentX + height + SC_DPI(6), baselineY - painter.fontMetrics().descent(), data.location);

//                currentX += width + SC_DPI(20);
            }

            if (data.author.isEmpty()) {
                ui->backgroundArtistLabel->setVisible(false);
            } else {
                ui->backgroundArtistLabel->setText(tr("by %1").arg(data.author));
                ui->backgroundArtistLabel->setVisible(true);
            }

            ui->backgroundInformation->setVisible(true);
        } else {
            ui->backgroundInformation->setVisible(false);
        }

        d->background = data;
        this->update();

        d->retrieving = false;
        if (d->retrieveAgain) {
            d->retrieveAgain = false;
            this->backgroundChanged();
        }
    })->error([ = ](QString error) {
        d->retrieving = false;
        if (d->retrieveAgain) {
            d->retrieveAgain = false;
            this->backgroundChanged();
        }
    });
}

void SystemSlide::resizeEvent(QResizeEvent* event) {
    d->hud->move(0, this->height() - d->hud->height());
    d->hud->setFixedWidth(this->width());
    this->backgroundChanged();
}

void SystemSlide::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    if (d->bgMode == DesktopBackground || d->bgMode == LockScreenBackground) {
        painter.setPen(Qt::transparent);
        painter.setBrush(Qt::black);
        painter.drawRect(0, 0, this->width(), this->height());

        if (!d->retrieving) {
            painter.drawPixmap(0, 0, d->background.px);
        }

        QLinearGradient darkener;
        darkener.setColorAt(0, QColor::fromRgb(0, 0, 0, 0));
        darkener.setColorAt(1, QColor::fromRgb(0, 0, 0, 200));

        darkener.setStart(0, this->height());
        darkener.setFinalStop(0, 0);
        painter.setBrush(darkener);
        painter.drawRect(0, 0, this->width(), this->height());

        painter.setPen(Qt::white);
    } else {
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 0, 0, 127));
        painter.drawRect(0, 0, this->width(), this->height());
    }
}

bool SystemSlide::eventFilter(QObject* watched, QEvent* event) {
    if (watched == this->parent()) {
        if (event->type() == QEvent::Resize) {
            this->resize(this->parentWidget()->size());
        }
    } else if (watched == d->hud) {
        if (event->type() == QEvent::Move) {
            d->coverWidget->setGeometry(0, d->hud->geometry().bottom(), this->width(), this->height() - d->hud->geometry().bottom());
        }
    } else if (watched == d->coverWidget) {
        if (event->type() == QEvent::Paint && d->dragResult) {
            QPainter painter(d->coverWidget);
            QPixmap px = d->dragResult->grab();
            painter.drawPixmap(QRect(QPoint(0, 0), px.size()), px);
        }
    }
    return false;
}

void SystemSlide::mousePressEvent(QMouseEvent* event) {
    this->activate();
}

void SystemSlide::mouseMoveEvent(QMouseEvent* event) {

}

void SystemSlide::mouseReleaseEvent(QMouseEvent* event) {

}
