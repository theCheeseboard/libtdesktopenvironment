#ifndef LOGINGREETER_H
#define LOGINGREETER_H

#include <QWidget>

namespace Ui {
    class LoginGreeter;
}

struct LoginGreeterPrivate;
class LoginGreeter : public QWidget {
        Q_OBJECT

    public:
        explicit LoginGreeter(QWidget* parent = nullptr);
        ~LoginGreeter();

        void setSessions(QMenu* sessions);

        void init(QString displayName, QString userName, bool isUnlock, QString defaultSession);
        void showPrompt(QString prompt, bool echo);
        void showMessage(QString message, bool error);
        void completeAuthentication();
        void reset();

    signals:
        void response(QString response);
        void loginComplete();
        void rejectLogin();

    private:
        Ui::LoginGreeter* ui;
        LoginGreeterPrivate* d;
        void changeSession(QString session);
};

#endif // LOGINGREETER_H
