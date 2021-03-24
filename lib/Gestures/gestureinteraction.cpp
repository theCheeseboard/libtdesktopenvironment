#include "gestureinteraction.h"

#include <QElapsedTimer>

struct GestureInteractionInstant {
    GestureTypes::GestureType type;
    GestureTypes::GestureDirection direction;
    double percentage;
    int fingers;
    GestureTypes::DeviceType deviceType;
    quint64 time;
};

struct GestureInteractionPrivate {
    QList<GestureInteractionInstant> instants;
    GestureInteractionInstant instant(quint64 time);
    QElapsedTimer currentTime;

    bool active = true;
};

GestureInteraction::GestureInteraction(QObject* parent) : QObject(parent) {
    d = new GestureInteractionPrivate();
    d->currentTime.start();
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

double GestureInteraction::percentage(quint64 time) {
    return d->instant(time).percentage;
}

double GestureInteraction::extrapolatePercentage(quint64 after) {
    return percentage() + velocity() * after;
}

int GestureInteraction::fingers() {
    return d->instants.last().fingers;
}

GestureTypes::DeviceType GestureInteraction::deviceType() {
    return d->instants.last().deviceType;
}

double GestureInteraction::velocity() {
    //Take data from the last 100 ms
    quint64 firstDataPoint = 0;
    if (lastDataTime() >= 100) firstDataPoint = lastDataTime() - 100;

    for (GestureInteractionInstant instant : d->instants) {
        if (instant.time > firstDataPoint) {
            //Use this for the data point
            GestureInteractionInstant lastInstant = d->instants.last();
            return (lastInstant.percentage - instant.percentage) / (lastInstant.time - instant.time);
        }
    }

    return 0;
}

QList<quint64> GestureInteraction::dataTimes() {
    QList<quint64> times;
    for (GestureInteractionInstant instant : d->instants) {
        times.append(instant.time);
    }
    return times;
}

quint64 GestureInteraction::currentTime() {
    return d->currentTime.elapsed();
}

quint64 GestureInteraction::lastDataTime() {
    return d->instants.last().time;
}

bool GestureInteraction::isActive() {
    return d->active;
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
    d->active = false;
    emit interactionEnded();
}

GestureInteractionInstant GestureInteractionPrivate::instant(quint64 time) {
    for (GestureInteractionInstant instant : instants) {
        if (instant.time == time) return instant;
    }
    return GestureInteractionInstant();
}
