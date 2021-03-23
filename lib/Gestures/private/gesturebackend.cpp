#include "gesturebackend.h"

GestureBackend::GestureBackend(QObject* parent) : QObject(parent) {

}

void GestureBackend::updateGesture(GestureInteractionPtr interaction, GestureTypes::GestureType type, GestureTypes::GestureDirection direction, double percentage, int fingers, GestureTypes::DeviceType deviceType, quint64 time) {
    interaction->gestureUpdate(type, direction, percentage, fingers, deviceType, time);
}

void GestureBackend::endGesture(GestureInteractionPtr interaction) {
    interaction->gestureEnd();
}
