#include "logingreeter.h"
#include "ui_logingreeter.h"

#include "messagepane.h"
#include "passwordpane.h"
#include "readypane.h"

struct LoginGreeterPrivate {
        QString displayName;
        QString userName;

        // Flag to check if the password pane was shown
        // If not, we show the welcome page at the end
        bool passwordPaneShown = false;

        // Flag to check if the prompt should be to unlock or log in
        bool isUnlock = false;

        QString defaultSession;

        QMenu* sessions;

        QList<QWidget*> panes;
};

LoginGreeter::LoginGreeter(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LoginGreeter) {
    ui->setupUi(this);
    d = new LoginGreeterPrivate();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    this->setEnabled(false);
}

LoginGreeter::~LoginGreeter() {
    delete d;
    delete ui;
}

void LoginGreeter::setSessions(QMenu* sessions) {
    d->sessions = sessions;
}

void LoginGreeter::init(QString displayName, QString userName, bool isUnlock, QString defaultSession) {
    for (auto pane : d->panes) {
        ui->stackedWidget->removeWidget(pane);
        pane->deleteLater();
    }
    d->panes.clear();

    this->reset();

    d->displayName = displayName;
    d->userName = userName;
    d->isUnlock = isUnlock;
    d->defaultSession = defaultSession;
}

void LoginGreeter::showPrompt(QString prompt, bool echo) {
    auto passwordPage = new PasswordPane(this);
    passwordPage->setSessions(d->sessions);
    passwordPage->prompt(prompt, d->userName, echo, d->isUnlock, d->defaultSession);

    connect(passwordPage, &PasswordPane::accept, this, [this](QString response) {
        emit this->response(response);
        this->setEnabled(false);
    });
    connect(passwordPage, &PasswordPane::reject, this, [this] {
        emit rejectLogin();
    });
    connect(passwordPage, &PasswordPane::sessionChanged, this, &LoginGreeter::changeSession);

    d->passwordPaneShown = true;
    ui->stackedWidget->addWidget(passwordPage);
    ui->stackedWidget->setCurrentWidget(passwordPage, ui->stackedWidget->count() != 1);
    d->panes.append(passwordPage);
    this->setEnabled(true);
}

void LoginGreeter::showMessage(QString message, bool error) {
    auto messagePage = new MessagePane(this);
    messagePage->setMessage(message, error);

    ui->stackedWidget->addWidget(messagePage);
    ui->stackedWidget->setCurrentWidget(messagePage, ui->stackedWidget->count() != 1);
    d->panes.append(messagePage);
    this->setEnabled(true);
}

void LoginGreeter::completeAuthentication() {
    if (d->passwordPaneShown) {
        // Start the session
        emit loginComplete();
    } else {
        // Ask the user for their session
        auto readyPage = new ReadyPane(this);
        readyPage->setSessions(d->sessions);
        ui->stackedWidget->setCurrentWidget(readyPage);
        readyPage->prompt(d->displayName, d->isUnlock, d->defaultSession);

        connect(readyPage, &ReadyPane::accept, this, [this] {
            emit loginComplete();
            this->setEnabled(false);
        });
        connect(readyPage, &ReadyPane::reject, this, [this] {
            emit rejectLogin();
        });
        connect(readyPage, &ReadyPane::sessionChanged, this, &LoginGreeter::changeSession);

        ui->stackedWidget->addWidget(readyPage);
        ui->stackedWidget->setCurrentWidget(readyPage, ui->stackedWidget->count() != 1);
        d->panes.append(readyPage);
        this->setEnabled(true);
    }
}

void LoginGreeter::reset() {
    this->setEnabled(false);
    d->passwordPaneShown = false;
}

void LoginGreeter::changeSession(QString session) {
}
