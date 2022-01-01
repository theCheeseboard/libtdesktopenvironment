#ifndef X11XSETTINGSPROVIDER_H
#define X11XSETTINGSPROVIDER_H

#include <QObject>

struct X11XSettingsProviderPrivate;

class X11XSettingsProvider : public QObject {
        Q_OBJECT
    public:
        explicit X11XSettingsProvider(QObject* parent = nullptr);
        ~X11XSettingsProvider();

        void setString(QString name, QString value);
        void setInt(QString name, quint32 value);
        void setColor(QString name, QColor value);

    signals:

    private:
        X11XSettingsProviderPrivate* d;

        void updateSetting();
};

#endif // X11XSETTINGSPROVIDER_H
