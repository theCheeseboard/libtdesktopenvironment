/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "waylandkeyboardtables.h"

#include "qwayland-tdesktopenvironment-keygrab-v1.h"

QList<quint32> TWayland::toEvdevCodes(Qt::Key key) {
    return keymap.values(key);
}

quint32 TWayland::toEvdevMod(Qt::KeyboardModifiers mod) {
    quint32 evdevMod = 0;
    if (mod & Qt::ShiftModifier) evdevMod |= TDESKTOPENVIRONMENT_KEYGRAB_MANAGER_V1_MODIFIER_SHIFT;
    if (mod & Qt::ControlModifier) evdevMod |= TDESKTOPENVIRONMENT_KEYGRAB_MANAGER_V1_MODIFIER_CONTROL;
    if (mod & Qt::AltModifier) evdevMod |= TDESKTOPENVIRONMENT_KEYGRAB_MANAGER_V1_MODIFIER_ALT;
    if (mod & Qt::MetaModifier) evdevMod |= TDESKTOPENVIRONMENT_KEYGRAB_MANAGER_V1_MODIFIER_SUPER;
    return evdevMod;
}

quint64 TWayland::evdevDescriptor(quint32 mod, quint32 key) {
    return (static_cast<quint64>(mod) << 32) | key;
}

void TWayland::breakoutEvdevDescriptor(quint64 descriptor, quint32* mod, quint32* key) {
    *mod = descriptor >> 32;
    *key = descriptor & 0xFF;
}
