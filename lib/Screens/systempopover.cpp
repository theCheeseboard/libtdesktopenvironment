#include "systempopover.h"

#include "private/overlaywindow.h"
#include <QApplication>
#include <QScreen>
#include <tscrim.h>

struct SystemPopoverPrivate {
        OverlayWindow* shownWindow;
        QList<OverlayWindow*> overlayWindows;
};

SystemPopover::SystemPopover(QWidget* popoverWidget, QObject* parent) :
    tPopover{popoverWidget, parent} {
    d = new SystemPopoverPrivate();
}

SystemPopover::~SystemPopover() {
    delete d;
}

void SystemPopover::show() {
    for (auto screen : qApp->screens()) {
        auto w = new OverlayWindow();
        w->setGeometry(screen->geometry());
        w->showFullScreen();

        connect(this, &SystemPopover::dismissed, w, [w] {
            w->close();
        });
        connect(this, &SystemPopover::destroyed, w, [w] {
            w->deleteLater();
        });

        if (screen->geometry().contains(QCursor::pos())) {
            tPopover::show(w);
            d->shownWindow = w;
        } else {
            auto scrim = tScrim::scrimForWidget(w);
            scrim->show();
        }
    }
}

void SystemPopover::dismiss() {
    for (auto w : d->overlayWindows) {
        if (w != d->shownWindow) tScrim::scrimForWidget(w)->hide();
    }
    tPopover::dismiss();
}
