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
#include "mprisplayer.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusConnectionInterface>

struct MprisPlayerPrivate {
    QDBusInterface *interface, *player;

    QMap<QString, QVariant> properties;
    QMap<QString, QString> propertyMappings;

//    bool canQuit, fullscreen, canFullscreen, canRaise, hasTrackList;
//    QString identity, desktopEntry;
//    MprisPlayer::PlayingStatus status;
};

MprisPlayerInterface::MprisPlayerInterface(QString service, QObject *parent) : QObject(parent)
{
    d = new MprisPlayerPrivate();
    d->interface = new QDBusInterface(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
    d->player = new QDBusInterface(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");

    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &MprisPlayerInterface::serviceOwnerChanged);
    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(dbusPropertyChanged(QString,QMap<QString, QVariant>,QStringList)));
    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Seeked", this, SIGNAL(seeked(qint64)));

    registerDbusProperty(d->interface, "canQuit", "CanQuit");
    registerDbusProperty(d->interface, "isFullscreen", "Fullscreen");
    registerDbusProperty(d->interface, "canFullscreen", "CanSetFullscreen");
    registerDbusProperty(d->interface, "canRaise", "CanRaise");
    registerDbusProperty(d->interface, "hasTrackList", "HasTrackList");
    registerDbusProperty(d->interface, "identity", "Identity");
    registerDbusProperty(d->interface, "desktopEntry", "DesktopEntry");

    registerDbusProperty(d->player, "playbackStatus", "PlaybackStatus");
    registerDbusProperty(d->player, "repeating", "LoopStatus");
    registerDbusProperty(d->player, "rate", "Rate");
    registerDbusProperty(d->player, "shuffle", "Shuffle");
    registerDbusProperty(d->player, "metadata", "Metadata");
    registerDbusProperty(d->player, "volume", "Volume");
    registerDbusProperty(d->player, "minRate", "MinimumRate");
    registerDbusProperty(d->player, "maxRate", "MaximumRate");
    registerDbusProperty(d->player, "canGoNext", "CanGoNext");
    registerDbusProperty(d->player, "canGoPrevious", "CanGoPrevious");
    registerDbusProperty(d->player, "canPlay", "CanPlay");
    registerDbusProperty(d->player, "canPause", "CanPause");
    registerDbusProperty(d->player, "canSeek", "CanSeek");
}

MprisPlayerInterface::~MprisPlayerInterface() {
    d->player->deleteLater();
    d->interface->deleteLater();
    delete d;
}

void MprisPlayerInterface::registerDbusProperty(QDBusInterface* interface, QString localProperty, QString remoteProperty) {
    QDBusMessage message = QDBusMessage::createMethodCall(interface->service(), interface->path(), "org.freedesktop.DBus.Properties", "Get");
    message.setArguments({
                             interface->interface(),
                             remoteProperty
                         });
    QDBusMessage reply = interface->connection().call(message);
//    QVariant propertyValue = interface->property(qPrintable(remoteProperty));
    QVariant propertyValue = reply.arguments().first().value<QDBusVariant>().variant();
    d->properties.insert(localProperty, propertyValue);
    d->propertyMappings.insert(interface->interface() + "." + remoteProperty, localProperty);
}

void MprisPlayerInterface::dbusPropertyChanged(QString interfaceName, QMap<QString, QVariant> changedProperties, QStringList invalidatedProperties) {
    Q_UNUSED(invalidatedProperties)
    for (QString property : changedProperties.keys()) {
        if (!d->propertyMappings.contains(interfaceName + "." + property)) {
            qDebug() << "MPRIS received unknown property change: " + interfaceName + "." + property;
            continue;
        }

        QString localProperty = d->propertyMappings.value(interfaceName + "." + property);
        d->properties.insert(localProperty, changedProperties.value(property));
        QMetaObject::invokeMethod(this, qPrintable(localProperty + "Changed"), Qt::DirectConnection);
    }
}

void MprisPlayerInterface::serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner)
{
    if (serviceName != d->interface->service()) return; //Not interested in this service
    if (oldOwner != "") {
        //We're gone!
        emit gone();
    }
}

QString MprisPlayerInterface::service() {
    return d->interface->service();
}

void MprisPlayerInterface::raise() {
    d->interface->asyncCall("Raise");
}

void MprisPlayerInterface::quit() {
    d->interface->asyncCall("Quit");
}

void MprisPlayerInterface::next()
{
    d->player->asyncCall("Next");
}

void MprisPlayerInterface::previous()
{
    d->player->asyncCall("Previous");
}

void MprisPlayerInterface::pause()
{
    d->player->asyncCall("Pause");
}

void MprisPlayerInterface::playPause()
{
    d->player->asyncCall("PlayPause");
}

void MprisPlayerInterface::stop()
{
    d->player->asyncCall("Stop");
}

void MprisPlayerInterface::play()
{
    d->player->asyncCall("Play");
}

void MprisPlayerInterface::seek(qint64 offset)
{
    d->player->asyncCall("Seek", offset);
}

void MprisPlayerInterface::setPosition(qint64 position)
{
    d->player->asyncCall("SetPosition", metadata().value("mpris:trackid"), position);
}

void MprisPlayerInterface::openUri(QString uri)
{
    d->player->asyncCall(uri);
}

QVariant MprisPlayerInterface::privateProperty(QString name) {
    return d->properties.value(name);
}

QString MprisPlayerInterface::identity() {
    return privateProperty("identity").toString();
}

bool MprisPlayerInterface::canQuit() {
    return privateProperty("canQuit").toBool();
}

bool MprisPlayerInterface::isFullscreen() {
    return privateProperty("isFullscreen").toBool();
}

bool MprisPlayerInterface::canFullscreen() {
    return privateProperty("canFullscreen").toBool();
}

bool MprisPlayerInterface::canRaise() {
    return privateProperty("canRaise").toBool();
}

bool MprisPlayerInterface::hasTrackList() {
    return privateProperty("hasTrackList").toBool();
}

void MprisPlayerInterface::setIsFullscreen(bool fullscreen) {
    d->interface->setProperty("Fullscreen", fullscreen);
}

QString MprisPlayerInterface::desktopEntry() {
    return privateProperty("desktopEntry").toString();
}

MprisPlayerInterface::PlayingStatus MprisPlayerInterface::playbackStatus() {
    QString status = privateProperty("playbackStatus").toString();
    if (status == "Playing") {
        return Playing;
    } else if (status == "Paused") {
        return Paused;
    } else {
        return Stopped;
    }
}

MprisPlayerInterface::RepeatStatus MprisPlayerInterface::repeating() {
    QString repeating = privateProperty("repeating").toString();
    if (repeating == "Track") {
        return RepeatOne;
    } else if (repeating == "Playlist") {
        return RepeatAll;
    } else {
        return NoRepeat;
    }
}

void MprisPlayerInterface::setRepeating(RepeatStatus repeating) {
    switch (repeating) {
        case RepeatOne:
            d->player->setProperty("LoopStatus", "Track");
            break;
        case RepeatAll:
            d->player->setProperty("LoopStatus", "Playlist");
            break;
        case NoRepeat:
            d->player->setProperty("LoopStatus", "None");
    }
}

double MprisPlayerInterface::rate() {
    return privateProperty("rate").toDouble();
}

void MprisPlayerInterface::setRate(double rate) {
    d->player->setProperty("Rate", rate);
}

bool MprisPlayerInterface::shuffle() {
    return privateProperty("shuffle").toBool();
}

void MprisPlayerInterface::setShuffle(bool shuffle) {
    d->player->setProperty("Shuffle", shuffle);
}

MetadataMap MprisPlayerInterface::metadata() {
    QVariant v = privateProperty("metadata");
    if (v.canConvert<MetadataMap>()) {
        return v.value<MetadataMap>();
    } else {
        //Marshal the DBus argument into a metadata map before adding it into the properties
        QDBusArgument arg = v.value<QDBusArgument>();
        MetadataMap map;
        arg >> map;
        return map;
    }
}

double MprisPlayerInterface::volume() {
    return privateProperty("volume").toDouble();
}

void MprisPlayerInterface::setVolume(double volume) {
    d->player->setProperty("Volume", volume);
}

qint64 MprisPlayerInterface::position() {
    return d->player->property("Position").toInt();
}

double MprisPlayerInterface::minRate()
{
    return privateProperty("minRate").toDouble();
}

double MprisPlayerInterface::maxRate()
{
    return privateProperty("maxRate").toDouble();
}

bool MprisPlayerInterface::canGoNext()
{
    return privateProperty("canGoNext").toBool();
}

bool MprisPlayerInterface::canGoPrevious()
{
    return privateProperty("canGoPrevious").toBool();
}

bool MprisPlayerInterface::canPlay()
{
    return privateProperty("canPlay").toBool();
}

bool MprisPlayerInterface::canPause()
{
    return privateProperty("canPause").toBool();
}

bool MprisPlayerInterface::canSeek()
{
    return privateProperty("canSeek").toBool();
}

bool MprisPlayerInterface::canControl()
{
    return d->player->property("CanControl").toBool();
}
