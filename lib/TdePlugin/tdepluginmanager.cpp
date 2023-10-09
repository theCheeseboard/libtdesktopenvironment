#include "tdepluginmanager.h"

#include "tdeplugininterface.h"
#include <QDir>
#include <QDirIterator>
#include <QPluginLoader>
#include <tlogger.h>

struct TdePluginManagerPrivate {
        TdePluginInterface* interface = nullptr;
};

TdePluginManager::TdePluginManager(QObject* parent) :
    QObject{parent} {
    d = new TdePluginManagerPrivate();
    this->loadPlugin();
}

TdePluginManager::~TdePluginManager() {
    delete d;
}

void TdePluginManager::loadPlugin() {
    QStringList searchPaths = qEnvironmentVariable("LD_LIBRARY_PATH").split(":", Qt::SkipEmptyParts);
    searchPaths.append("/usr/local/lib");
    searchPaths.append("/usr/lib");
    searchPaths.removeAll("");

    for (const auto& searchPath : searchPaths) {
        QDirIterator iterator(searchPath + "/libtdesktopenvironment/plugins", {"*.so"}, QDir::NoFilter, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            auto loader = new QPluginLoader(iterator.next());
            if (auto instance = qobject_cast<TdePluginInterface*>(loader->instance())) {
                if (instance->supportedOnThisPlatform()) {
                    instance->activate();
                    d->interface = instance;
                    return;
                }
            }
        }
    }

    tCritical("TdePluginManager") << "Unable to find a suitable backend plugin for libtdesktopenvironment.";
    tCritical("TdePluginManager") << "This application will probably crash now.";
}

TdePluginInterface* TdePluginManager::loadedInterface() {
    static auto instance = new TdePluginManager();
    return instance->d->interface;
}
