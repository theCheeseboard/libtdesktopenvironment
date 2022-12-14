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
#ifndef READYPANE_H
#define READYPANE_H

#include <QWidget>

namespace Ui {
    class ReadyPane;
}

struct ReadyPanePrivate;
class ReadyPane : public QWidget {
        Q_OBJECT

    public:
        explicit ReadyPane(QWidget* parent = nullptr);
        ~ReadyPane();

        void prompt(QString username, bool isUnlock, QString defaultSession);

        void setSessions(QMenu* menu);

    signals:
        void sessionChanged(QString session);
        void accept();
        void reject();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_loginButton_clicked();

        void on_sessionSelect_triggered(QAction* arg1);

    private:
        Ui::ReadyPane* ui;
        ReadyPanePrivate* d;
};

#endif // READYPANE_H
