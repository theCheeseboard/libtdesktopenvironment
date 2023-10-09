#ifndef X11PLUGIN_H
#define X11PLUGIN_H

#include <QObject>
#include "TdePlugin/tdeplugininterface.h"

struct WaylandPluginPrivate;
class WaylandPlugin : public QObject, public TdePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TdePluginInterface_iid FILE "waylandplugin.json")
    Q_INTERFACES(TdePluginInterface)
public:
    explicit WaylandPlugin(QObject *parent = nullptr);

signals:

private:
    WaylandPluginPrivate* d;

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
