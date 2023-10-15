#ifndef TWAYLANDREGISTRY_H
#define TWAYLANDREGISTRY_H

#include <QApplication>
#include <QCoroGenerator>
#include <QList>
#include <qpa/qplatformnativeinterface.h>
#include <wayland-client-protocol.h>

template<typename T>
concept QtWaylandClientProtocol = requires(T a, struct ::wl_registry* registry, int id, int version) {
    { a.init(registry, id, version) } -> std::same_as<void>;
    { a.isInitialized() } -> std::same_as<bool>;
    { T::interface() } -> std::same_as<const struct ::wl_interface*>;
};

class tWaylandRegistry {
    public:
        tWaylandRegistry() {
            auto display = reinterpret_cast<wl_display*>(qApp->platformNativeInterface()->nativeResourceForIntegration("display"));

            wl_registry_listener listener = {
                [](void* data, wl_registry* registry, quint32 name, const char* interface, quint32 version) {
                auto* self = static_cast<tWaylandRegistry*>(data);
                self->items.append({registry, name, QString::fromLocal8Bit(interface), version});
                },
                [](void* data, wl_registry* registry, quint32 name) {
                Q_UNUSED(data)
                Q_UNUSED(registry)
                Q_UNUSED(name)
            }};

            registry = wl_display_get_registry(display);
            wl_registry_add_listener(registry, &listener, this);
            wl_display_roundtrip(display);
        }

        ~tWaylandRegistry() {
            wl_registry_destroy(registry);
        }

        template<QtWaylandClientProtocol T> bool init(T* client) {
            auto interface = T::interface();
            auto interfaceName = QString::fromLocal8Bit(interface->name);
            for (const auto& item : this->items) {
                if (item.interface == interfaceName) {
                    client->init(item.registry, item.name, item.version);
                    return true;
                }
            }
            return false;
        }

        QSharedPointer<wl_seat> seat() {
            return this->bind<wl_seat>(&wl_seat_interface, 1);
        }

        template<typename T> QSharedPointer<T> bind(const wl_interface* interface, quint32 version) {
            auto interfaceName = QString::fromLocal8Bit(interface->name);
            for (const auto& item : this->items) {
                if (item.interface == interfaceName) {
                    auto bound = static_cast<T*>(wl_registry_bind(item.registry, item.name, interface, version));
                    return QSharedPointer<T>(bound, &tWaylandRegistry::resourceDeleter<T>);
                }
            }
            return nullptr;
        }

        template<typename T> QCoro::Generator<QSharedPointer<T>> interfaces(const wl_interface* interface, quint32 version) {
            auto interfaceName = QString::fromLocal8Bit(interface->name);
            for (const auto& item : this->items) {
                if (item.interface == interfaceName) {
                    auto bound = static_cast<T*>(wl_registry_bind(item.registry, item.name, interface, version));
                    co_yield QSharedPointer<T>(bound, &tWaylandRegistry::resourceDeleter<T>);
                }
            }
        }

    private:
        struct RegistryItem {
                wl_registry* registry;
                quint32 name;
                QString interface;
                quint32 version;
        };

        QList<RegistryItem> items;
        wl_registry* registry;

        template<typename T> static void resourceDeleter(T* obj) {
        }
};

#endif // TWAYLANDREGISTRY_H
