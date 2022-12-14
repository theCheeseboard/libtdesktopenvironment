#include "messagepane.h"
#include "ui_messagepane.h"

#include <QMessageBox>

MessagePane::MessagePane(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MessagePane) {
    ui->setupUi(this);
}

MessagePane::~MessagePane() {
    delete ui;
}

void MessagePane::setMessage(QString message, bool error) {
    ui->messageLabel->setText(message);
}
