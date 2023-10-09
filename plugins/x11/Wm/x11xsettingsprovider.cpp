#include "x11xsettingsprovider.h"

#include "Screens/screendaemon.h"
#include <QColor>
#include <QMap>
#include <tx11info.h>

#include "x11functions.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>

template<typename T> QByteArray stringToXSettingProperty(QString string) {
    QByteArray data;
    QByteArray utf8String = string.toUtf8();

    T stringLength = utf8String.length();

    data.append(reinterpret_cast<char*>(&stringLength), sizeof(stringLength));
    data.append(utf8String);

    // Pad out the rest with bytes
    int pad = (4 - (stringLength % 4)) % 4;
    data.append(pad, 0);

    return data;
}

struct XSetting {
        enum SettingType : char {
            Integer = 0,
            String = 1,
            Colour = 2
        };

        SettingType type;
        QString name;
        quint32 lastChangeSerial;

        quint32 intValue;
        QString stringValue;
        QColor colorValue;

        QByteArray exportToProperty() {
            QByteArray data;

            data.append(type);
            data.append(static_cast<char>(0)); // -
            data.append(stringToXSettingProperty<quint16>(name));
            data.append(reinterpret_cast<char*>(&lastChangeSerial), sizeof(lastChangeSerial));

            switch (type) {
                case XSetting::Integer:
                    data.append(reinterpret_cast<char*>(&intValue), sizeof(intValue));
                    break;
                case XSetting::String:
                    data.append(stringToXSettingProperty<quint32>(stringValue));
                    break;
                case XSetting::Colour:
                    quint16 r, g, b, a;
                    r = colorValue.red();
                    g = colorValue.green();
                    b = colorValue.blue();
                    a = colorValue.alpha();

                    data.append(reinterpret_cast<char*>(&r), sizeof(r));
                    data.append(reinterpret_cast<char*>(&g), sizeof(g));
                    data.append(reinterpret_cast<char*>(&b), sizeof(b));
                    data.append(reinterpret_cast<char*>(&a), sizeof(a));
                    break;
            }

            return data;
        }
};

struct X11XSettingsProviderPrivate {
        Window settingsWindow = 0;
        quint32 serial = 0;

        QMap<QString, XSetting> settings;
};

X11XSettingsProvider::X11XSettingsProvider(QObject* parent) :
    QObject{parent} {
    d = new X11XSettingsProviderPrivate();

    setString("Gtk/CursorThemeName", "contemporary_cursors");
    setInt("Gtk/CursorThemeSize", 24);
    setString("Gtk/FontName", "Contemporary 10");

    connect(ScreenDaemon::instance(), &ScreenDaemon::dpiChanged, this, [this] {
        setInt("Xft/DPI", ScreenDaemon::instance()->dpi() * 1024);
    });
    setInt("Xft/DPI", ScreenDaemon::instance()->dpi() * 1024);
}

X11XSettingsProvider::~X11XSettingsProvider() {
    XDestroyWindow(tX11Info::display(), d->settingsWindow);
    delete d;
}

void X11XSettingsProvider::setString(QString name, QString value) {
    d->serial++;

    XSetting setting;
    setting.type = XSetting::String;
    setting.name = name;
    setting.lastChangeSerial = d->serial;
    setting.stringValue = value;
    d->settings.insert(name, setting);

    updateSetting();
}

void X11XSettingsProvider::setInt(QString name, quint32 value) {
    d->serial++;

    XSetting setting;
    setting.type = XSetting::Integer;
    setting.name = name;
    setting.lastChangeSerial = d->serial;
    setting.intValue = value;
    d->settings.insert(name, setting);

    updateSetting();
}

void X11XSettingsProvider::setColor(QString name, QColor value) {
    d->serial++;

    XSetting setting;
    setting.type = XSetting::Colour;
    setting.name = name;
    setting.lastChangeSerial = d->serial;
    setting.colorValue = value;
    d->settings.insert(name, setting);

    updateSetting();
}

void X11XSettingsProvider::setAsSettingsManager() {
    Atom xsettingsAtom = XInternAtom(tX11Info::display(), "_XSETTINGS_S0", true);

    d->settingsWindow = XCreateSimpleWindow(tX11Info::display(), tX11Info::appRootWindow(), 0, 0, 1, 1, 1, 1, 1);
    XSetSelectionOwner(tX11Info::display(), xsettingsAtom, d->settingsWindow, CurrentTime);

    TX11::sendMessageToRootWindow("MANAGER", d->settingsWindow, CurrentTime, xsettingsAtom, d->settingsWindow);
    updateSetting();
}

void X11XSettingsProvider::updateSetting() {
    if (!d->settingsWindow) return;

    QByteArray propertyValue;

    propertyValue.append(static_cast<char>(0)); // byte-order
    propertyValue.append(static_cast<char>(0)); // -
    propertyValue.append(static_cast<char>(0)); // -
    propertyValue.append(static_cast<char>(0)); // -

    quint32 settingsLength = d->settings.size();
    propertyValue.append(reinterpret_cast<char*>(&d->serial), sizeof(d->serial));
    propertyValue.append(reinterpret_cast<char*>(&settingsLength), sizeof(settingsLength));

    for (XSetting setting : d->settings.values()) {
        propertyValue.append(setting.exportToProperty());
    }

    Atom property = XInternAtom(tX11Info::display(), "_XSETTINGS_SETTINGS", true);
    XChangeProperty(tX11Info::display(), d->settingsWindow, property, property, 8, PropModeReplace, reinterpret_cast<const unsigned char*>(propertyValue.constData()), propertyValue.length());
}
