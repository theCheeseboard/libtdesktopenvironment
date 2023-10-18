#include "waylandoutput.h"

#include <QApplication>
#include <qpa/qplatformnativeinterface.h>

#include <tlogger.h>

struct WaylandOutputPrivate {
        QString name;
        QString description;
};

WaylandOutput::WaylandOutput(::wl_output* output, QObject* parent) :
    QObject{parent}, QtWayland::wl_output(output) {
    d = new WaylandOutputPrivate();

    auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    wl_display_roundtrip(display);
}

WaylandOutput::~WaylandOutput() {
    this->release();
    delete d;
}

QString WaylandOutput::name() {
    return d->name;
}

QString WaylandOutput::description() {
    return d->description;
}

void WaylandOutput::output_name(const QString& name) {
    d->name = name;
}

void WaylandOutput::output_description(const QString& description) {
    d->description = description;
}

void WaylandOutput::output_done() {
}

void WaylandOutput::output_geometry(int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const QString& make, const QString& model, int32_t transform) {
}

void WaylandOutput::output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
}

void WaylandOutput::output_scale(int32_t factor) {
}
