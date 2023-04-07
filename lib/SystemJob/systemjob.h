#ifndef SYSTEMJOB_H
#define SYSTEMJOB_H

#include <QCoroTask>
#include <QDBusConnection>
#include <QDBusInterface>

class SystemJobController;
struct SystemJobPrivate;
class SystemJob : public QDBusInterface {
        Q_OBJECT
    public:
        explicit SystemJob(QDBusConnection connection, QString service, QString path, SystemJobController* parent = nullptr);
        ~SystemJob();

        QString service();
        bool valid();

        QCoro::Task<quint64> progress();
        QCoro::Task<quint64> totalProgress();
        QCoro::Task<QString> state();
        QCoro::Task<QString> title();
        QCoro::Task<QString> status();

    signals:
        void progressChanged(quint64 progress);
        void totalProgressChanged(quint64 totalProgress);
        void stateChanged(QString state);
        void titleChanged(QString state);
        void statusChanged(QString state);
        void validChanged(bool valid);

    private:
        SystemJobPrivate* d;
};

typedef QSharedPointer<SystemJob> SystemJobPtr;

#endif // SYSTEMJOB_H
