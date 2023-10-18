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
        void output_geometry(int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const QString& make, const QString& model, int32_t transform);
        void output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        void output_scale(int32_t factor);
};

#endif // WAYLANDOUTPUT_H
