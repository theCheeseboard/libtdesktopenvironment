/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
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
#include <QtTest>
#include <QApplication>

// add necessary includes here
#include <Wm/desktopwm.h>

class TestX11Backend : public QObject
{
        Q_OBJECT

    public:
        TestX11Backend();
        ~TestX11Backend();

    private slots:
        void test_case1();

};

TestX11Backend::TestX11Backend()
{

}

TestX11Backend::~TestX11Backend()
{

}

void TestX11Backend::test_case1()
{
    connect(DesktopWm::instance(), &DesktopWm::windowAdded, this, [=](DesktopWmWindowPtr window) {
        qDebug() << "Window added:" << window->title();
    });
    connect(DesktopWm::instance(), &DesktopWm::windowRemoved, this, [=](DesktopWmWindowPtr window) {
        qDebug() << "Window gone:" << window->title();
    });
    connect(DesktopWm::instance(), &DesktopWm::activeWindowChanged, this, [=]() {
        if (DesktopWm::activeWindow()) {
            qDebug() << "Active window changed:" << DesktopWm::activeWindow()->title();
        } else {
            qDebug() << "Active window changed: nothing";
        }
    });
    connect(DesktopWm::instance(), &DesktopWm::currentDesktopChanged, this, [=]() {
        qDebug() << "Current desktop changed:" << DesktopWm::desktops().at(DesktopWm::currentDesktop());
    });
    connect(DesktopWm::instance(), &DesktopWm::desktopCountChanged, this, [=]() {
        qDebug() << "Current desktops:" << DesktopWm::desktops();
    });
    qDebug() << DesktopWm::desktops();
    QApplication::instance()->exec();
}

QTEST_MAIN(TestX11Backend)

#include "tst_testx11backend.moc"
