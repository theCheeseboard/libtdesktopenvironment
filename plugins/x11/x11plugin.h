#ifndef X11PLUGIN_H
#define X11PLUGIN_H

#include <QObject>
#include "TdePlugin/tdeplugininterface.h"

struct X11PluginPrivate;
class X11Plugin : public QObject, public TdePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TdePluginInterface_iid FILE "x11plugin.json")
    Q_INTERFACES(TdePluginInterface)
public:
    explicit X11Plugin(QObject *parent = nullptr);

signals:

private:
    X11PluginPrivate* d;

    // TdePluginInterface interface
public:
    void activate();
    void deactivate();
    bool supportedOnThisPlatform();
    WmBackend *wmBackend();
    ScreenBackend* screenBackend();
    GestureBackend* gestureBackend();
};

#endif // X11PLUGIN_H
