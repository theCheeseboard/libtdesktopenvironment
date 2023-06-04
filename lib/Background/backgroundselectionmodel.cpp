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
#include "backgroundselectionmodel.h"

#include "backgroundcontroller.h"
#include <QPainter>

struct BackgroundSelectionModelPrivate {
        BackgroundController* bg;

        QMap<int, QPixmap> px;
        QList<int> loadingPx;

        int oldRowCount = 0;
};

BackgroundSelectionModel::BackgroundSelectionModel(QObject* parent) :
    QAbstractListModel(parent) {
    d = new BackgroundSelectionModelPrivate();
    d->bg = new BackgroundController(BackgroundController::Desktop);
    connect(d->bg, &BackgroundController::currentBackgroundChanged, this, &BackgroundSelectionModel::emitDataChanged);
    connect(d->bg, &BackgroundController::availableWallpapersChanged, this, [this](int newWallpapers) {
        beginInsertRows(QModelIndex(), d->oldRowCount, d->oldRowCount + newWallpapers);
        endInsertRows();
    });
}

BackgroundSelectionModel::~BackgroundSelectionModel() {
    delete d;
}

int BackgroundSelectionModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    int c = d->bg->availableBackgrounds().count();
    if (d->oldRowCount != c) {
        d->px.clear();
        d->loadingPx.clear();
    }
    d->oldRowCount = c;
    return c;
}

QVariant BackgroundSelectionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    QString bgName = d->bg->availableBackgrounds().at(index.row());
    if (role == Qt::DecorationRole) {
        if (d->px.contains(index.row())) {
            return d->px.value(index.row());
        } else if (!d->loadingPx.contains(index.row())) {
            d->loadingPx.append(index.row());

            if (bgName == "community" || bgName == "custom") {
                QString text;
                if (bgName == "community") text = tr("Community Backgrounds");
                if (bgName == "custom") text = tr("Custom");

                QPixmap communityPx(QSize(213, 120));
                communityPx.fill(QColor(0, 0, 0, 127));

                QPainter painter(&communityPx);
                QFont fnt;
                fnt.setPointSize(12);
                painter.setFont(fnt);
                painter.setPen(Qt::white);
                painter.drawText(QRect(QPoint(0, 0), communityPx.size()), Qt::AlignCenter | Qt::TextWordWrap, text);
                painter.end();

                d->px.insert(index.row(), communityPx);
                return communityPx;
            } else {
                d->bg->getBackground(bgName, QSize(213, 120)).then([this, index](BackgroundController::BackgroundData data) {
                    d->px.insert(index.row(), data.px);
                    QTimer::singleShot(0, this, &BackgroundSelectionModel::emitDataChanged);
                });
            }
        }
        return QPixmap();
    } else if (role == Qt::UserRole) {
        return bgName;
    } else if (role == Qt::UserRole + 1) {
        return d->bg->currentBackgroundName(BackgroundController::Desktop) == bgName;
    } else if (role == Qt::UserRole + 2) {
        return d->bg->currentBackgroundName(BackgroundController::LockScreen) == bgName;
    }
    return QVariant();
}

void BackgroundSelectionModel::emitDataChanged() {
    emit dataChanged(index(0), index(rowCount()));
}

BackgroundSelectionDelegate::BackgroundSelectionDelegate() :
    QStyledItemDelegate(nullptr) {
}

void BackgroundSelectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->drawPixmap(option.rect, index.data(Qt::DecorationRole).value<QPixmap>());

    bool isDesktop = index.data(Qt::UserRole + 1).toBool();
    bool isLock = index.data(Qt::UserRole + 2).toBool();

    if (isDesktop || isLock) {
        QRect squareRect;
        squareRect.setSize(SC_DPI_T(QSize(24, 24), QSize));
        squareRect.moveRight(option.rect.right() - SC_DPI(8));
        squareRect.moveBottom(option.rect.bottom() - SC_DPI(8));

        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(squareRect);

        QIcon icon;
        if (isDesktop && isLock) {
            icon = QIcon::fromTheme("dialog-ok");
        } else if (isDesktop) {
            icon = QIcon::fromTheme("video-display");
        } else {
            icon = QIcon::fromTheme("system-lock-screen");
        }

        QRect iconRect;
        iconRect.setSize(SC_DPI_T(QSize(16, 16), QSize));
        iconRect.moveCenter(squareRect.center());
        painter->drawPixmap(iconRect, icon.pixmap(iconRect.size()));
    }
}

QSize BackgroundSelectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    return SC_DPI_T(QSize(213, 120), QSize);
}
