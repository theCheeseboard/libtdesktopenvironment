/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#ifndef PASSWORDPANE_H
#define PASSWORDPANE_H

#include <QWidget>

namespace Ui {
    class PasswordPane;
}

class QAbstractListModel;
struct PasswordPanePrivate;
class PasswordPane : public QWidget {
        Q_OBJECT

    public:
        explicit PasswordPane(QWidget* parent = nullptr);
        ~PasswordPane();

        void prompt(QString prompt, QString username, bool echo, bool isUnlock, QString defaultSession);

        void setSessions(QMenu* menu);

    signals:
        void sessionChanged(QString session);
        void accept(QString response);
        void reject();

    private slots:
        void on_unlockButton_clicked();

        void on_titleLabel_backButtonClicked();

        void on_sessionSelect_triggered(QAction* arg1);

    private:
        Ui::PasswordPane* ui;
        PasswordPanePrivate* d;
};

#endif // PASSWORDPANE_H
