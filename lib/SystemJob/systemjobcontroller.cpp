#include "systemjobcontroller.h"

#include "systemjob.h"
#include <QCoroDBus>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QTimer>

struct SystemJobControllerPrivate {
        QDBusConnection connection;
        QMap<QString, QString> friendlyInterfaces;
        QMap<QString, QDBusInterface*> managerInterfaces;
        QMultiMap<QString, SystemJobPtr> jobs;
};

SystemJobController::SystemJobController(QDBusConnection dbusConnection, QObject* parent) :
    QObject{parent} {
    d = new SystemJobControllerPrivate{
        dbusConnection};

    connect(dbusConnection.interface(), &QDBusConnectionInterface::NameOwnerChanged, this, &SystemJobController::serviceOwnerChanged);

    for (const auto& name : dbusConnection.interface()->registeredServiceNames().value()) {
        if (!name.startsWith("com.vicr123.libcontemporary.tjob.")) continue;
        d->friendlyInterfaces.insert(dbusConnection.interface()->serviceOwner(name).value(), name);
        registerManager(name);
    }
}

SystemJobController::~SystemJobController() {
    delete d;
}

QList<SystemJobPtr> SystemJobController::jobs() {
    return d->jobs.values();
}

QString SystemJobController::applicationNameForService(QString service) {
    if (!d->managerInterfaces.contains(service)) return "";
    return d->managerInterfaces.value(service)->property("ApplicationName").toString();
}

QString SystemJobController::desktopEntryForService(QString service) {
    if (!d->managerInterfaces.contains(service)) return "";
    return d->managerInterfaces.value(service)->property("ApplicationDesktopEntry").toString();
}

QString SystemJobController::serviceForDesktopEntry(QString desktopEntry) {
    for (auto interface : qAsConst(d->managerInterfaces)) {
        if (interface->property("ApplicationDesktopEntry").toString() == desktopEntry) {
            return interface->service();
        }
    }
    return "";
}

void SystemJobController::jobAdded(QDBusObjectPath objectPath, const QDBusMessage& message) {
    this->registerJob(d->friendlyInterfaces.value(message.service()), objectPath.path());
}

void SystemJobController::serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner) {
    if (!serviceName.startsWith("com.vicr123.libcontemporary.tjob.")) return;
    if (oldOwner != "") {
        d->friendlyInterfaces.remove(oldOwner);
        deregisterManager(serviceName);
    }
    if (newOwner != "") {
        d->friendlyInterfaces.insert(newOwner, serviceName);
        registerManager(serviceName);
    }
}

QCoro::Task<> SystemJobController::registerManager(QString serviceName) {
    auto interface = new QDBusInterface(serviceName, "/com/vicr123/libcontemporary/tjob", "com.vicr123.libcontemporary.tjob.Manager", d->connection);
    // clang-format off
    d->connection.connect(serviceName, "/com/vicr123/libcontemporary/tjob", "com.vicr123.libcontemporary.tjob.Manager", "JobAdded", this, SLOT(jobAdded(QDBusObjectPath,QDBusMessage)));
    // clang-format on
    d->managerInterfaces.insert(serviceName, interface);

    auto jobs = co_await interface->asyncCall("Jobs");
    QList<QDBusObjectPath> jobPaths;
    if (!jobs.arguments().isEmpty()) {
        auto argument = jobs.arguments().constFirst().value<QDBusArgument>();
        argument >> jobPaths;
        for (const auto& job : jobPaths) {
            registerJob(serviceName, job.path());
        }
    }
}

void SystemJobController::deregisterManager(QString serviceName) {
    auto interface = d->managerInterfaces.take(serviceName);
    d->jobs.remove(serviceName);
    interface->deleteLater();
}

void SystemJobController::registerJob(QString serviceName, QString path) {
    auto job = SystemJobPtr(new SystemJob(d->connection, serviceName, path, this));
    d->jobs.insert(serviceName, job);

    QTimer::singleShot(0, this, [this, job] {
        emit newJob(job);
    });
}
