#include "systemjob.h"

#include "systemjobcontroller.h"
#include <QCoroDBus>
#include <QDBusConnectionInterface>
#include <QDBusInterface>

struct SystemJobPrivate {
        static const char* interface;
        QString service;
        bool valid = true;
};

const char* SystemJobPrivate::interface = "com.vicr123.libcontemporary.tjob.Job";

SystemJob::SystemJob(QDBusConnection connection, QString service, QString path, SystemJobController* parent) :
    QDBusInterface(service, path, SystemJobPrivate::interface, connection, parent) {
    d = new SystemJobPrivate();
    d->service = service;

    connection.connect(service, path, SystemJobPrivate::interface, "ProgressChanged", this, SIGNAL(progressChanged(quint64)));
    connection.connect(service, path, SystemJobPrivate::interface, "TotalProgressChanged", this, SIGNAL(totalProgressChanged(quint64)));
    connection.connect(service, path, SystemJobPrivate::interface, "StateChanged", this, SIGNAL(stateChanged(QString)));
    connection.connect(service, path, SystemJobPrivate::interface, "TitleChanged", this, SIGNAL(titleChanged(QString)));
    connection.connect(service, path, SystemJobPrivate::interface, "StatusChanged", this, SIGNAL(statusChanged(QString)));

    connect(connection.interface(), &QDBusConnectionInterface::NameOwnerChanged, this, [this, service](QString serviceName, QString oldOwner, QString newOwner) {
        if (serviceName != service) return;
        if (oldOwner != "") {
            // Invalidate this
            d->valid = false;
            emit validChanged(false);
        }
    });
}

SystemJob::~SystemJob() {
    delete d;
}

QString SystemJob::service() {
    return d->service;
}

bool SystemJob::valid() {
    return d->valid;
}

QCoro::Task<quint64> SystemJob::progress() {
    auto reply = co_await asyncCall("Progress");
    if (reply.arguments().isEmpty()) co_return static_cast<quint64>(0);
    co_return reply.arguments().constFirst().toULongLong();
}

QCoro::Task<quint64> SystemJob::totalProgress() {
    auto reply = co_await asyncCall("TotalProgress");
    if (reply.arguments().isEmpty()) co_return static_cast<quint64>(0);
    co_return reply.arguments().constFirst().toULongLong();
}

QCoro::Task<QString> SystemJob::state() {
    auto reply = co_await asyncCall("State");
    if (reply.arguments().isEmpty()) co_return "";
    co_return reply.arguments().constFirst().toString();
}

QCoro::Task<QString> SystemJob::title() {
    auto reply = co_await asyncCall("Title");
    if (reply.arguments().isEmpty()) co_return "";
    co_return reply.arguments().constFirst().toString();
}

QCoro::Task<QString> SystemJob::status() {
    auto reply = co_await asyncCall("Status");
    if (reply.arguments().isEmpty()) co_return "";
    co_return reply.arguments().constFirst().toString();
}
