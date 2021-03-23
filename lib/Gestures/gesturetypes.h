#ifndef GESTURETYPES_H
#define GESTURETYPES_H

namespace GestureTypes {
    enum GestureType {
        Swipe,
        Pinch
    };

    enum GestureDirection {
        Up,
        Left,
        Right,
        Down,
        In,
        Out
    };

    enum DeviceType {
        Touchpad,
        TouchScreen
    };
}

#endif // GESTURETYPES_H
