/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef WAYLANDSCREEN_H
#define WAYLANDSCREEN_H

#include "Screens/systemscreen.h"
#include "qwayland-wlr-gamma-control-unstable-v1.h"
#include "qwayland-wlr-output-management-unstable-v1.h"

class WaylandScreenBackend;
struct WaylandScreenPrivate;
class WaylandScreen : public SystemScreen,
                      public QtWayland::zwlr_output_head_v1,
                      public QtWayland::zwlr_gamma_control_v1 {
        Q_OBJECT
    public:
        explicit WaylandScreen(::zwlr_output_head_v1* head, WaylandScreenBackend* backend);
        ~WaylandScreen();

    signals:

    private:
        WaylandScreenPrivate* d;

        void normaliseScreens();
        void updateGammaRamps();

        // zwlr_output_head_v1 interface
    protected:
        void zwlr_output_head_v1_name(const QString& name);
        void zwlr_output_head_v1_description(const QString& description);
        void zwlr_output_head_v1_physical_size(int32_t width, int32_t height);
        void zwlr_output_head_v1_mode(zwlr_output_mode_v1* mode);
        void zwlr_output_head_v1_enabled(int32_t enabled);
        void zwlr_output_head_v1_current_mode(zwlr_output_mode_v1* mode);
        void zwlr_output_head_v1_position(int32_t x, int32_t y);
        void zwlr_output_head_v1_transform(int32_t transform);
        void zwlr_output_head_v1_scale(wl_fixed_t scale);
        void zwlr_output_head_v1_finished();
        void zwlr_output_head_v1_make(const QString& make);
        void zwlr_output_head_v1_model(const QString& model);
        void zwlr_output_head_v1_serial_number(const QString& serial_number);

        // SystemScreen interface
    public:
        bool isScreenBrightnessAvailable();
        double screenBrightness();
        void setScreenBrightness(double screenBrightness);
        void adjustGammaRamps(QString adjustmentName, GammaRamps ramps);
        void removeGammaRamps(QString adjustmentName);
        bool powered() const;
        bool isPrimary() const;
        void setPowered(bool powered);
        QRect geometry() const;
        void move(QPoint topLeft);
        QList<Mode> availableModes() const;
        int currentMode() const;
        void setCurrentMode(int mode);
        void setAsPrimary();
        Rotation currentRotation() const;
        void setRotation(Rotation rotation);
        QString displayName() const;
        QString physicalMonitorId() const;
        QByteArray edid() const;
        QScreen* qtScreen() const;
        void set();
        void reset();
        QString manufacturer() const;
        QString productName() const;
        QString restoreKey() const;
};

#endif // WAYLANDSCREEN_H
