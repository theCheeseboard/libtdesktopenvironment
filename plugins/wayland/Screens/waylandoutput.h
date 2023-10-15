#ifndef WAYLANDOUTPUT_H
#define WAYLANDOUTPUT_H

#include "qwayland-wayland.h"
#include <QObject>

struct WaylandOutputPrivate;
class WaylandOutput : public QObject,
                      public QtWayland::wl_output {
        Q_OBJECT
    public:
        explicit WaylandOutput(::wl_output* output, QObject* parent = nullptr);
        ~WaylandOutput();

        QString name();
        QString description();

    signals:

    private:
        WaylandOutputPrivate* d;

        // wl_output interface
    protected:
        void output_name(const QString& name);
        void output_description(const QString& description);
        void output_done();
};

#endif // WAYLANDOUTPUT_H
