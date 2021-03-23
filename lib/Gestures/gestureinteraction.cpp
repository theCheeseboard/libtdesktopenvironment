#include "gestureinteraction.h"

struct GestureInteractionInstant {
    GestureTypes::GestureType type;
    GestureTypes::GestureDirection direction;
    double percentage;
    int fingers;
    GestureTypes::DeviceType deviceType;
    qint64 time;
};

struct GestureInteractionPrivate {
    QList<GestureInteractionInstant> instants;
};

GestureInteraction::GestureInteraction(QObject* parent) : QObject(parent) {
    d = new GestureInteractionPrivate();
}

GestureInteraction::~GestureInteraction() {
    delete d;
}

GestureTypes::GestureType GestureInteraction::gestureType() {
    return d->instants.last().type;
}

GestureTypes::GestureDirection GestureInteraction::gestureDirection() {
    return d->instants.last().direction;
}

double GestureInteraction::percentage() {
    return d->instants.last().percentage;
}

int GestureInteraction::fingers() {
    return d->instants.last().fingers;
}

GestureTypes::DeviceType GestureInteraction::deviceType() {
    return d->instants.last().deviceType;
}

bool GestureInteraction::isValidInteraction(GestureTypes::GestureType type, GestureTypes::GestureDirection direction, int fingers) {
    return gestureType() == type && gestureDirection() == direction && this->fingers() == fingers;
}

void GestureInteraction::gestureUpdate(GestureTypes::GestureType type, GestureTypes::GestureDirection direction, double percentage, int fingers, GestureTypes::DeviceType deviceType, quint64 time) {
    GestureInteractionInstant instant;
    instant.type = type;
    instant.direction = direction;
    instant.percentage = percentage;
    instant.fingers = fingers;
    instant.deviceType = deviceType;
    instant.time = time;
    d->instants.append(instant);

    emit interactionUpdated();
}

void GestureInteraction::gestureEnd() {
    emit interactionEnded();
}
