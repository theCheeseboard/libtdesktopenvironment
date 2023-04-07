#ifndef SYSTEMJOBCONTROLLER_H
#define SYSTEMJOBCONTROLLER_H

#include "systemjob.h"
#include <QCoroTask>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

struct SystemJobControllerPrivate;
class SystemJobController : public QObject {
        Q_OBJECT
    public:
        explicit SystemJobController(QDBusConnection dbusConnection, QObject* parent = nullptr);
        ~SystemJobController();

        QList<SystemJobPtr> jobs();
        QString applicationNameForService(QString service);
        QString desktopEntryForService(QString service);
        QString serviceForDesktopEntry(QString desktopEntry);

    signals:
        void newJob(SystemJobPtr job);

    private slots:
        void jobAdded(QDBusObjectPath objectPath, const QDBusMessage& message);

    private:
        SystemJobControllerPrivate* d;

        void serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner);
        QCoro::Task<> registerManager(QString serviceName);
        void deregisterManager(QString serviceName);
        void registerJob(QString serviceName, QString path);
};

#endif // SYSTEMJOBCONTROLLER_H
