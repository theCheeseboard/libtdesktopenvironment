#ifndef MESSAGEPANE_H
#define MESSAGEPANE_H

#include <QWidget>

namespace Ui {
    class MessagePane;
}

class MessagePane : public QWidget {
        Q_OBJECT

    public:
        explicit MessagePane(QWidget* parent = nullptr);
        ~MessagePane();

        void setMessage(QString message, bool error);

    private:
        Ui::MessagePane* ui;
};

#endif // MESSAGEPANE_H
