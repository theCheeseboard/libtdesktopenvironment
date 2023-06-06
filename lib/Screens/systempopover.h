#ifndef SYSTEMPOPOVER_H
#define SYSTEMPOPOVER_H

#include <tpopover.h>

struct SystemPopoverPrivate;
class SystemPopover : public tPopover {
        Q_OBJECT
    public:
        explicit SystemPopover(QWidget* popoverWidget, QObject* parent = nullptr);
        ~SystemPopover();

    signals:

    public slots:
        void show();
        void dismiss();

    private:
        SystemPopoverPrivate* d;
};

#endif // SYSTEMPOPOVER_H
