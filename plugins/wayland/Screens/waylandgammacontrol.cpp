#include "waylandgammacontrol.h"

#include "twaylandregistry.h"
#include "waylandoutput.h"
#include <tlogger.h>

struct WaylandGammaControlPrivate {
        QString name;
        QString description;

        tWaylandRegistry registry;
        QSharedPointer<wl_output> output;

        quint32 gammaSize;
};

WaylandGammaControl::WaylandGammaControl(QString name, QString description, QtWayland::zwlr_gamma_control_manager_v1* gammaControlManager, QObject* parent) :
    QObject{parent} {
    d = new WaylandGammaControlPrivate();
    d->name = name;
    d->description = description;

    for (const auto& wlOutput : d->registry.interfaces<wl_output>(&wl_output_interface, 4)) {
        WaylandOutput output(wlOutput.data());
        tDebug("WaylandGammaControl") << "Found output   name: " << output.name();
        if (output.name() == name && output.description() == description) {
            // We found the correct output
            tDebug("WaylandGammaControl") << "Found the output for a gamma control";
            d->output = wlOutput;
            auto gammaControl = gammaControlManager->get_gamma_control(wlOutput.data());
            this->QtWayland::zwlr_gamma_control_v1::init(gammaControl);
            break;
        }
    }

    auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    wl_display_roundtrip(display);
}

WaylandGammaControl::~WaylandGammaControl() {
    this->destroy();

    auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    wl_display_roundtrip(display);

    delete d;
}

quint32 WaylandGammaControl::rampSize() {
    return d->gammaSize;
}

bool WaylandGammaControl::isReady() {
    return this->isInitialized();
}

void WaylandGammaControl::setGamma(int32_t fd) {
    this->set_gamma(fd);

    auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));
    wl_display_roundtrip(display);
}

void WaylandGammaControl::zwlr_gamma_control_v1_gamma_size(uint32_t size) {
    d->gammaSize = size;
}

void WaylandGammaControl::zwlr_gamma_control_v1_failed() {
    tWarn("WaylandGammaControl") << "Setting gamma ramps failed for display " << d->name;
}
