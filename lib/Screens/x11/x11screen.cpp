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
#include "x11screen.h"

#include <QDebug>
#include <QX11Info>
#include <X11/extensions/Xrandr.h>
#include "Wm/x11/x11functions.h"

#include <math.h>

struct X11ScreenPrivate {
    RROutput output;
    double brightness;
    long brightnessMin, brightnessMax;

    static bool backlightAtomSet;
    static Atom backlightAtom;

    QMap<QString, SystemScreen::GammaRamps> gammaRamps;
};

bool X11ScreenPrivate::backlightAtomSet = false;
Atom X11ScreenPrivate::backlightAtom = None;

template <typename T> struct OutputProperty {
    typedef T* iterator;

    Atom type;
    int format;
    ulong nItems;
    ulong nBytesRemain;
    T* data;

    ~OutputProperty() {
        //Automatically free the data
        if (this->data != nullptr) {
            XFree(reinterpret_cast<void*>(this->data));
        }
    }

    QString typeName() {
        return TX11::atomName(type);
    }
    iterator begin() {
        return data;
    }
    iterator end() {
        return data + nItems;
    }
    bool contains(T item) {
        for (T i : *this) {
            if (i == item) return true;
        }
        return false;
    }
    T first() {
        return *data;
    }
    T at(int index) {
        return data[index];
    }
    T at(long index) {
        return data[index];
    }
    T operator[](int index) {
        return data[index];
    }
    T operator->() {
        return *data;
    }
    T* operator+(int other) {
        return data + other;
    }
    T* operator+(long other) {
        return data + other;
    }

    template<typename U> operator OutputProperty<U>() {
        OutputProperty<U> prop;
        prop.data = this->data;
        return prop;
    }
};


X11Screen::X11Screen(RROutput output, QObject* parent) : SystemScreen(parent) {
    d = new X11ScreenPrivate();
    d->output = output;

    if (!d->backlightAtomSet) {
        d->backlightAtom = XInternAtom(QX11Info::display(), "backlight", true);
        if (d->backlightAtom == None) d->backlightAtom = XInternAtom(QX11Info::display(), "BACKLIGHT", true);
        d->backlightAtomSet = true;
    }
}

X11Screen::~X11Screen() {
    delete d;
}

void X11Screen::updateScreen() {
    updateBrightness();
}

void X11Screen::updateBrightness() {
    if (d->backlightAtom == None) {
        d->brightness = -1;
        return;
    }

    OutputPropertyPtr<qint32> backlightProperty = getOutputProperty<qint32>(d->backlightAtom, XA_INTEGER, 0, 4);
    if (!backlightProperty) {
        d->brightness = -1;
        return;
    }

    XRRPropertyInfo* info = XRRQueryOutputProperty(QX11Info::display(), d->output, d->backlightAtom);
    d->brightnessMin = info->values[0];
    d->brightnessMax = info->values[1];
    d->brightness = static_cast<double>(*backlightProperty->data - d->brightnessMin) / d->brightnessMax;

    emit screenBrightnessChanged(d->brightness);
}

void X11Screen::updateGammaRamps() {
    GammaRamps ramps;
    if (d->gammaRamps.count() == 0) {
        //Reset the gamma
        ramps.red = 1;
        ramps.green = 1;
        ramps.blue = 1;
    } else {
        //Interpolate all the gamma values
        QList<GammaRamps> allRamps = d->gammaRamps.values();
        ramps = allRamps.first();
        for (auto i = allRamps.begin() + 1; i != allRamps.end(); i++) {
            ramps.red *= i->red;
            ramps.green *= i->green;
            ramps.blue *= i->blue;
        }
    }

    //Set the gamma ramps
    XRRCrtcGamma* gamma = XRRAllocGamma(gammaRampSize());

    for (int i = 0; i < gamma->size; i++) {
        double factor = static_cast<double>(UINT16_MAX + 1) * i / gamma->size;
        gamma->red[i] = static_cast<quint16>(factor * ramps.red + 0.5);
        gamma->green[i] = static_cast<quint16>(factor * ramps.green + 0.5);
        gamma->blue[i] = static_cast<quint16>(factor * ramps.blue + 0.5);
    }

    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo* info = XRRGetOutputInfo(QX11Info::display(), resources, d->output);
    XRRSetCrtcGamma(QX11Info::display(), info->crtc, gamma);

    XRRFreeGamma(gamma);
    XRRFreeOutputInfo(info);
    XRRFreeScreenResources(resources);
}


bool X11Screen::isScreenBrightnessAvailable() {
    return d->brightness >= 0;
}

double X11Screen::screenBrightness() {
    return d->brightness;
}

void X11Screen::setScreenBrightness(double screenBrightness) {
    if (screenBrightness < 0) screenBrightness = 0;
    if (screenBrightness > 1) screenBrightness = 1;
    qint32 brightnessLevel = static_cast<qint32>((d->brightnessMax - d->brightnessMin) * screenBrightness + d->brightnessMin);
    XRRChangeOutputProperty(QX11Info::display(), d->output, d->backlightAtom, XA_INTEGER, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&brightnessLevel), 1);
}

void X11Screen::removeGammaRamps(QString adjustmentName) {
    d->gammaRamps.remove(adjustmentName);
    updateGammaRamps();
}

int X11Screen::gammaRampSize() {
    int size;
    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo* info = XRRGetOutputInfo(QX11Info::display(), resources, d->output);

    size = XRRGetCrtcGammaSize(QX11Info::display(), info->crtc);

    XRRFreeOutputInfo(info);
    XRRFreeScreenResources(resources);

    return size;
}

void X11Screen::adjustGammaRamps(QString adjustmentName, SystemScreen::GammaRamps ramps) {
    d->gammaRamps.insert(adjustmentName, ramps);
    updateGammaRamps();
}

template<typename T> OutputPropertyPtr<T> X11Screen::getOutputProperty(Atom property, Atom type, long offset, long length) {
    OutputPropertyPtr<T> prop(new OutputProperty<T>());

    Atom typeReturn;
    int formatReturn;
    unsigned long nItems, nBytesRemain;
    unsigned char* data;

    XRRGetOutputProperty(QX11Info::display(),
        d->output,
        property,
        offset,
        length,
        false,
        0,
        type,
        &typeReturn,
        &formatReturn,
        &nItems,
        &nBytesRemain,
        &data);

    if (!data) return nullptr; //Couldn't get output property

    prop->type = typeReturn;
    prop->format = formatReturn;
    prop->nItems = nItems;
    prop->nBytesRemain = nBytesRemain;
    prop->data = reinterpret_cast<T*>(data);

    return prop;
}
