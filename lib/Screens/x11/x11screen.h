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
#ifndef X11SCREEN_H
#define X11SCREEN_H

#include "../systemscreen.h"

typedef unsigned long XID;
typedef XID RROutput;
typedef XID Atom;

struct X11ScreenPrivate;
template <typename T> struct OutputProperty;
template<typename T> using OutputPropertyPtr = QSharedPointer<OutputProperty<T>>;
class X11Screen : public SystemScreen {
        Q_OBJECT
    public:
        explicit X11Screen(RROutput output, QObject* parent = nullptr);
        ~X11Screen();

        bool isScreenBrightnessAvailable();
        double screenBrightness();
        void setScreenBrightness(double screenBrightness);

        void adjustGammaRamps(QString adjustmentName, GammaRamps ramps);
        void removeGammaRamps(QString adjustmentName);
        int gammaRampSize();

        void updateScreen();

    signals:

    private:
        X11ScreenPrivate* d;

        void updateBrightness();
        void updateGammaRamps();

        template<typename T> OutputPropertyPtr<T> getOutputProperty(Atom property, Atom type, long offset = 0, long length = ~0L);
};

#endif // X11SCREEN_H
