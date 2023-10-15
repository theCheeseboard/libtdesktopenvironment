#ifndef WAYLANDGAMMACONTROL_H
#define WAYLANDGAMMACONTROL_H

#include "qwayland-wlr-gamma-control-unstable-v1.h"
#include <QObject>

struct WaylandGammaControlPrivate;
class WaylandGammaControl : public QObject,
                            private QtWayland::zwlr_gamma_control_v1 {
        Q_OBJECT
    public:
        explicit WaylandGammaControl(QString name, QString description, QtWayland::zwlr_gamma_control_manager_v1* gammaControlManager, QObject* parent = nullptr);
        ~WaylandGammaControl();

        quint32 rampSize();

        bool isReady();
        void setGamma(int32_t fd);

    signals:

    private:
        WaylandGammaControlPrivate* d;

        // zwlr_gamma_control_v1 interface
    protected:
        void zwlr_gamma_control_v1_gamma_size(uint32_t size);
        void zwlr_gamma_control_v1_failed();
};

#endif // WAYLANDGAMMACONTROL_H
