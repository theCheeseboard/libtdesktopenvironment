#ifndef TDEPLUGINMANAGER_H
#define TDEPLUGINMANAGER_H

#include <QObject>
#include "tdeplugininterface.h"

struct TdePluginManagerPrivate;
class TdePluginManager : public QObject
{
    Q_OBJECT
public:
    ~TdePluginManager() override;

    static TdePluginInterface* loadedInterface();

signals:

private:
    explicit TdePluginManager(QObject *parent = nullptr);
    TdePluginManagerPrivate* d;

    void loadPlugin();
};

#endif // TDEPLUGINMANAGER_H
