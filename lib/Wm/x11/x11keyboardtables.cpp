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

#include "x11keyboardtables.h"

KeySym TX11::toKeySym(Qt::Key key) {
    QString keystr = QKeySequence(key).toString().toLower();
    KeySym ks = XStringToKeysym(qPrintable(QKeySequence(key).toString().toLower()));
    if (ks != NoSymbol) return ks;

    for (int i = 0; KeyTbl[i] != 0; i += 2) {
        if (KeyTbl[i + 1] == key) return KeyTbl[i];
    }
    return NoSymbol;
}

quint32 TX11::toNativeModifiers(Qt::KeyboardModifiers modifiers) {
    quint32 flag = 0;
    if (modifiers & Qt::ShiftModifier) flag |= ShiftMask;
    if (modifiers & Qt::ControlModifier) flag |= ControlMask;
    if (modifiers & Qt::AltModifier) flag |= Mod1Mask;
    if (modifiers & Qt::MetaModifier) flag |= Mod4Mask;
    return flag;
}
