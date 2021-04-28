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

#include <QMap>
#include <QX11Info>
#include <QScreen>
#include <QApplication>
#include <QCryptographicHash>
#include <X11/extensions/Xrandr.h>
#include "Wm/x11/x11functions.h"
#include "../screendaemon.h"

#include <math.h>

struct X11ScreenPrivate {
    RROutput output;
    double brightness;
    long brightnessMin, brightnessMax;

    static bool backlightAtomSet;
    static Atom backlightAtom;
    static Atom edidAtom;

    bool powered = false;
    bool isPrimary = false;
    QRect geometry;
    QList<SystemScreen::Mode> modes;
    int currentMode = 0;
    Rotation currentRotation = 0;
    QString name;

    QMap<QString, SystemScreen::GammaRamps> gammaRamps;
};

bool X11ScreenPrivate::backlightAtomSet = false;
Atom X11ScreenPrivate::backlightAtom = None;
Atom X11ScreenPrivate::edidAtom = None;

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

        d->edidAtom = XInternAtom(QX11Info::display(), "EDID", true);
        if (d->edidAtom == None) d->edidAtom = XInternAtom(QX11Info::display(), "EDID_DATA", true);

        d->backlightAtomSet = true;
    }
}

X11Screen::~X11Screen() {
    delete d;
}

void X11Screen::updateScreen() {
    updateBrightness();

    //Update available modes
    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo* info = XRRGetOutputInfo(QX11Info::display(), resources, d->output);
    QMap<RRMode, XRRModeInfo> modes;
    for (int i = 0; i < resources->nmode; i++) {
        modes.insert(resources->modes[i].id, resources->modes[i]);
    }
    QList<Mode> availableModes;
    for (int i = 0; i < info->nmode; i++) {
        Mode mode;
        XRRModeInfo modeInfo = modes.value(info->modes[i]);

        mode.width = modeInfo.width;
        mode.height = modeInfo.height;

        int vTotal = modeInfo.vTotal;
        if (modeInfo.modeFlags & RR_DoubleScan) vTotal *= 2;
        if (modeInfo.modeFlags & RR_Interlace) vTotal /= 2;
        mode.framerate = static_cast<double>(modeInfo.dotClock) / (modeInfo.hTotal * vTotal);

        mode.isInterlaced = modeInfo.modeFlags & RR_Interlace;
        mode.id = modeInfo.id;

        availableModes.append(mode);
    }
    d->modes = availableModes;
    d->name = QString::fromLocal8Bit(info->name);

    emit availableModesChanged(availableModes);

    //Update geometry and power status
    if (info->crtc) {
        XRRCrtcInfo* crtc = XRRGetCrtcInfo(QX11Info::display(), resources, info->crtc);
        d->geometry = QRect(crtc->x, crtc->y, crtc->width, crtc->height);
        d->currentMode = crtc->mode;
        d->currentRotation = crtc->rotation;
        XRRFreeCrtcInfo(crtc);

        d->powered = true;
    } else {
        d->powered = false;
    }

    emit poweredChanged(d->powered);
    emit rotationChanged(currentRotation());
    emit geometryChanged(geometry());
    emit currentModeChanged(d->currentMode);

    d->isPrimary = XRRGetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow()) == d->output;
    emit isPrimaryChanged(d->isPrimary);

    XRRFreeOutputInfo(info);
    XRRFreeScreenResources(resources);
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

void X11Screen::normaliseScreens() {
    QRect bounds(0, 0, 0, 0);

    //Find out how far we should offset all of the screens
    for (SystemScreen* screen : ScreenDaemon::instance()->screens()) {
        X11Screen* scr = static_cast<X11Screen*>(screen);
        if (!scr->powered()) continue;

        bounds = bounds.united(scr->geometry());
    }

    //Offset all of the screens
    for (SystemScreen* screen : ScreenDaemon::instance()->screens()) {
        X11Screen* scr = static_cast<X11Screen*>(screen);
        if (!scr->powered()) continue;

        scr->d->geometry.translate(-bounds.topLeft());
        emit scr->geometryChanged(scr->geometry());
    }
}

void X11Screen::set() {
    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo* info = XRRGetOutputInfo(QX11Info::display(), resources, d->output);

    if (d->isPrimary && XRRGetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow()) != d->output) {
        XRRSetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow(), d->output);
    }

    if (info->crtc == 0) {
        if (d->powered) {
            //Find a suitable CRTC for this output
            RRCrtc crtc = None;
            for (int i = 0; i < info->ncrtc; i++) {
                struct XRRCrtcInfoDeleter {
                    static inline void cleanup(XRRCrtcInfo* pointer) {
                        XRRFreeCrtcInfo(pointer);
                    }
                };

                QScopedPointer<XRRCrtcInfo, XRRCrtcInfoDeleter> crtcInfo(XRRGetCrtcInfo(QX11Info::display(), resources, info->crtcs[i]));
                if (crtcInfo->noutput > 0) {
                    //This CRTC is already being used, but let's check if we can clone the displays
                    if (crtcInfo->mode != static_cast<RRMode>(d->currentMode)) continue;
                    if (crtcInfo->x != d->geometry.left()) continue;
                    if (crtcInfo->y != d->geometry.top()) continue;
                    if (crtcInfo->rotation != d->currentRotation) continue;

                    //We can use this CRTC
                    crtc = info->crtcs[i];
                    break;
                } else {
                    //This CRTC is unused, so we can use this CRTC
                    crtc = info->crtcs[i];
                    break;
                }
            }

            if (crtc != None) {
                //Configure the output on this CRTC
                XRRSetCrtcConfig(QX11Info::display(), resources, crtc, CurrentTime, d->geometry.left(), d->geometry.top(), d->currentMode, d->currentRotation, &d->output, 1);
            }
        } else {
            //Do nothing; the screen isn't powered and doesn't need to be powered
            return;
        }
    } else {
        if (d->powered) {
            //Adjust this CRTC
            XRRSetCrtcConfig(QX11Info::display(), resources, info->crtc, CurrentTime, d->geometry.left(), d->geometry.top(), d->currentMode, d->currentRotation, &d->output, 1);
        } else {
            //Turn off this CRTC
            XRRSetCrtcConfig(QX11Info::display(), resources, info->crtc, CurrentTime, 0, 0, None, RR_Rotate_0, nullptr, 0);
        }
    }

    XRRFreeOutputInfo(info);
    XRRFreeScreenResources(resources);

    //Set the screen size properly
    ScreenDaemon::instance()->setDpi(ScreenDaemon::instance()->dpi());
}

void X11Screen::reset() {
    this->updateScreen();
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

bool X11Screen::powered() const {
    return d->powered;
}

bool X11Screen::isPrimary() const {
    return d->isPrimary;
}

void X11Screen::setPowered(bool powered) {
    d->powered = powered;
    normaliseScreens();
    emit poweredChanged(powered);
}

QRect X11Screen::geometry() const {
//    QRect geometry = d->geometry;
//    return geometry;
    return d->geometry;
}

void X11Screen::move(QPoint topLeft) {
    d->geometry.moveTopLeft(topLeft);

    normaliseScreens();

//    emit geometryChanged(geometry());
}

QList<SystemScreen::Mode> X11Screen::availableModes() const {
    return d->modes;
}

int X11Screen::currentMode() const {
    return d->currentMode;
}

void X11Screen::setCurrentMode(int mode) {
    d->currentMode = mode;
    for (Mode modeSpec : d->modes) {
        if (mode == modeSpec.id) {
            d->geometry.setSize(QSize(modeSpec.width, modeSpec.height));
            if (currentRotation() == Portrait || currentRotation() == UpsideDownPortrait) d->geometry = d->geometry.transposed();
            break;
        }
    }
}

void X11Screen::setAsPrimary() {
    SystemScreen* screen = ScreenDaemon::instance()->primayScreen();
    if (screen == this) return;
    if (screen) {
        X11Screen* x11Screen = static_cast<X11Screen*>(screen);
        x11Screen->d->isPrimary = false;
        emit x11Screen->isPrimaryChanged(false);
    }

    d->isPrimary = true;
    emit this->isPrimaryChanged(true);
}

SystemScreen::Rotation X11Screen::currentRotation() const {
    if (d->currentRotation & RR_Rotate_0) {
        return Landscape;
    } else if (d->currentRotation & RR_Rotate_90) {
        return Portrait;
    } else if (d->currentRotation & RR_Rotate_180) {
        return UpsideDown;
    } else if (d->currentRotation & RR_Rotate_270) {
        return UpsideDownPortrait;
    }
    return Portrait;
}

void X11Screen::setRotation(SystemScreen::Rotation rotation) {
    SystemScreen::Rotation oldRotation = currentRotation();

    switch (rotation) {
        case SystemScreen::Landscape:
            d->currentRotation = RR_Rotate_0;
            break;
        case SystemScreen::Portrait:
            d->currentRotation = RR_Rotate_90;
            break;
        case SystemScreen::UpsideDown:
            d->currentRotation = RR_Rotate_180;
            break;
        case SystemScreen::UpsideDownPortrait:
            d->currentRotation = RR_Rotate_270;
            break;
    }

    emit rotationChanged(currentRotation());

    if (((oldRotation == Landscape || oldRotation == UpsideDown) && (rotation == Portrait || rotation == UpsideDownPortrait)) ||
        ((oldRotation == Portrait || oldRotation == UpsideDownPortrait) && (rotation == Landscape || rotation == UpsideDown))) {
        //We need to transpose the geometry of the screen
        d->geometry = d->geometry.transposed();
        emit geometryChanged(geometry());
    }

    normaliseScreens();
}

QString X11Screen::displayName() const {
    QScreen* scr = this->qtScreen();
    if (scr) {
        return scr->manufacturer() + " " + scr->model();
    } else {
        return d->name;
    }
}

QString X11Screen::physicalMonitorId() const {
    QByteArray edid = this->edid();
    if (edid.count() > 0) return QCryptographicHash::hash(edid, QCryptographicHash::Sha256).toHex();

    QScreen* scr = this->qtScreen();
    if (scr) {
        return QCryptographicHash::hash(scr->manufacturer().append(scr->model()).append(scr->serialNumber()).toUtf8(), QCryptographicHash::Sha256).toHex();
    } else {
        return d->name;
    }
}

QByteArray X11Screen::edid() const {
    OutputPropertyPtr<qint8> edidProperty = getOutputProperty<qint8>(d->edidAtom, XA_INTEGER, 0, 4);

    QByteArray edid;
    if (edidProperty && edidProperty->nItems > 0) {
        edid = QByteArray(reinterpret_cast<const char*>(edidProperty->data), edidProperty->nBytesRemain);
    }
    return edid;
}

QScreen* X11Screen::qtScreen() const {
    for (QScreen* screen : QApplication::screens()) {
        if (screen->name() == d->name) return screen;
    }
    return nullptr;
}

void X11Screen::adjustGammaRamps(QString adjustmentName, SystemScreen::GammaRamps ramps) {
    d->gammaRamps.insert(adjustmentName, ramps);
    updateGammaRamps();
}

template<typename T> OutputPropertyPtr<T> X11Screen::getOutputProperty(Atom property, Atom type, long offset, long length) const {
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
