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
#ifndef X11FUNCTIONS_H
#define X11FUNCTIONS_H

#include <QSharedPointer>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

namespace TX11 {
    QString atomName(Atom atom);

    template <typename T> struct WindowProperty {
        typedef T* iterator;

        Atom type;
        int format;
        ulong nItems;
        ulong nBytesRemain;
        T* data = nullptr;

        ~WindowProperty() {
            //Automatically free the data
            if (this->data != nullptr) {
                XFree(reinterpret_cast<void*>(this->data));
            }
        }

        QString typeName() {
            return atomName(type);
        }
        iterator begin() {
            return data;
        }
        iterator end() {
            return data + nItems;
        }
        bool contains(T item) {
            for (T i : *this) {
                if (i == item) return true;
            }
            return false;
        }
        T first() {
            return *data;
        }
        T at(int index) {
            return data[index];
        }
        T at(long index) {
            return data[index];
        }
        T operator[](int index) {
            return data[index];
        }
        T operator->() {
            return *data;
        }
        T* operator+(int other) {
            return data + other;
        }
        T* operator+(long other) {
            return data + other;
        }

        template<typename U> operator WindowProperty<U>() {
            WindowProperty<U> prop;
            prop.data = this->data;
            return prop;
        }
    };
    template<typename T> using WindowPropertyPtr = QSharedPointer<WindowProperty<T>>;

    template<typename T> WindowPropertyPtr<T> getWindowProperty(QString property, Window window, Atom type, long offset = 0, long length = ~0L) {
        TX11::WindowPropertyPtr<T> prop(new TX11::WindowProperty<T>());

        Atom typeReturn;
        int formatReturn;
        unsigned long nItems, nBytesRemain;
        unsigned char *data;


        XGetWindowProperty(QX11Info::display(),
                           window,
                           XInternAtom(QX11Info::display(), qPrintable(property), true),
                           offset,
                           length,
                           false,
                           type,
                           &typeReturn,
                           &formatReturn,
                           &nItems,
                           &nBytesRemain,
                           &data);

        prop->type = typeReturn;
        prop->format = formatReturn;
        prop->nItems = nItems;
        prop->nBytesRemain = nBytesRemain;
        prop->data = reinterpret_cast<T*>(data);

        return prop;
    }


    template<typename T> WindowPropertyPtr<T> getWindowProperty(QString property, Window window, QString type, long offset = 0, long length = ~0L) {
        return getWindowProperty<T>(property, window, XInternAtom(QX11Info::display(), qPrintable(type), true), offset, length);
    }

    template<typename T> WindowPropertyPtr<T> getRootWindowProperty(QString property, Atom type, long offset = 0, long length = ~0L) {
        return getWindowProperty<T>(property, QX11Info::appRootWindow(), type, offset, length);
    };

    template<typename T> WindowPropertyPtr<T> getRootWindowProperty(QString property, QString type, long offset = 0, long length = ~0L) {
        return getRootWindowProperty<T>(property, XInternAtom(QX11Info::display(), qPrintable(type), true), offset, length);
    }

    void sendMessageToRootWindow(QString message, Window window, long data0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);
}

#endif // X11FUNCTIONS_H
