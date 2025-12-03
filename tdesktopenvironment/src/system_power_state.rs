use crate::dbus_generated::accounts::accounts::AccountsProxy;
use crate::dbus_generated::register_dbus_listener;
use crate::dbus_generated::systemd_logind::manager::ManagerProxy;
use gpui::{AsyncApp, Context, WeakEntity};
use std::rc::Rc;
use std::time::Duration;
use zbus::Connection;
use zbus::proxy::CacheProperties;

#[derive(Copy, Clone, Default)]
pub enum SystemPowerStateCapability {
    Available,
    AvailableWithAuthentication,
    NoPermissions,

    #[default]
    NotAvailable,
}

impl<E> TryFrom<Result<String, E>> for SystemPowerStateCapability {
    type Error = ();

    fn try_from(value: Result<String, E>) -> Result<Self, Self::Error> {
        match value {
            Ok(value) => match value.as_str() {
                "na" => Ok(SystemPowerStateCapability::NotAvailable),
                "yes" => Ok(SystemPowerStateCapability::Available),
                "no" => Ok(SystemPowerStateCapability::NoPermissions),
                "challenge" => Ok(SystemPowerStateCapability::AvailableWithAuthentication),
                _ => Err(()),
            },
            Err(_) => Err(()),
        }
    }
}

macro_rules! query_login1_state {
  ($login1_proxy:ident, $($t:ident),*) => {
    $(let $t = SystemPowerStateCapability::try_from($login1_proxy.$t().await).unwrap_or_default();)*
  }
}

pub struct SystemPowerState {
    login1_proxy: Option<Rc<ManagerProxy<'static>>>,
    can_power_off: SystemPowerStateCapability,
    can_reboot: SystemPowerStateCapability,
    can_suspend: SystemPowerStateCapability,
    can_hibernate: SystemPowerStateCapability,
    can_hybrid_sleep: SystemPowerStateCapability,
    can_reboot_to_firmware_setup: SystemPowerStateCapability,
    can_reboot_to_boot_loader_menu: SystemPowerStateCapability,
    can_reboot_to_boot_loader_entry: SystemPowerStateCapability,
}

impl SystemPowerState {
    pub fn new(cx: &mut Context<Self>) -> Self {
        cx.spawn(
            async move |weak_entity: WeakEntity<Self>, cx: &mut AsyncApp| {
                let Ok(connection) = Connection::system().await else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let Ok(login1_proxy) = ManagerProxy::builder(&connection)
                    .cache_properties(CacheProperties::Yes)
                    .build()
                    .await
                else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let login1_proxy = Rc::new(login1_proxy);

                if weak_entity
                    .update(cx, |this, cx| {
                        this.login1_proxy = Some(login1_proxy);
                        this.update_capability(cx);
                        cx.notify();
                    })
                    .is_err()
                {
                    return;
                }
            },
        )
        .detach();

        Self {
            login1_proxy: None,
            can_power_off: Default::default(),
            can_reboot: Default::default(),
            can_suspend: Default::default(),
            can_hibernate: Default::default(),
            can_hybrid_sleep: Default::default(),
            can_reboot_to_firmware_setup: Default::default(),
            can_reboot_to_boot_loader_menu: Default::default(),
            can_reboot_to_boot_loader_entry: Default::default(),
        }
    }

    pub fn update_capability(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(
                async move |weak_this: WeakEntity<Self>, cx: &mut AsyncApp| {
                    query_login1_state!(
                        login1_proxy,
                        can_power_off,
                        can_reboot,
                        can_suspend,
                        can_hibernate,
                        can_hybrid_sleep,
                        can_reboot_to_firmware_setup,
                        can_reboot_to_boot_loader_menu,
                        can_reboot_to_boot_loader_entry
                    );

                    let _ = weak_this.update(cx, |this, cx| {
                        this.can_power_off = can_power_off;
                        this.can_reboot = can_reboot;
                        this.can_suspend = can_suspend;
                        this.can_hibernate = can_hibernate;
                        this.can_hybrid_sleep = can_hybrid_sleep;
                        this.can_reboot_to_firmware_setup = can_reboot_to_firmware_setup;
                        this.can_reboot_to_boot_loader_menu = can_reboot_to_boot_loader_menu;
                        this.can_reboot_to_boot_loader_entry = can_reboot_to_boot_loader_entry;
                        cx.notify();
                    });
                },
            )
            .detach()
        }
    }

    pub fn can_power_off(&self) -> SystemPowerStateCapability {
        self.can_power_off
    }

    pub fn can_reboot(&self) -> SystemPowerStateCapability {
        self.can_reboot
    }

    pub fn can_suspend(&self) -> SystemPowerStateCapability {
        self.can_suspend
    }

    pub fn can_hibernate(&self) -> SystemPowerStateCapability {
        self.can_hibernate
    }

    pub fn can_hybrid_sleep(&self) -> SystemPowerStateCapability {
        self.can_hybrid_sleep
    }

    pub fn can_reboot_to_firmware_setup(&self) -> SystemPowerStateCapability {
        self.can_reboot_to_firmware_setup
    }

    pub fn can_reboot_to_boot_loader_menu(&self) -> SystemPowerStateCapability {
        self.can_reboot_to_boot_loader_menu
    }

    pub fn can_reboot_to_boot_loader_entry(&self) -> SystemPowerStateCapability {
        self.can_reboot_to_boot_loader_entry
    }

    pub fn reboot_to_firmware_setup(&self) -> Option<bool> {
        self.login1_proxy.as_ref().and_then(|login1_proxy| {
            login1_proxy
                .cached_reboot_to_firmware_setup()
                .ok()
                .flatten()
        })
    }

    pub fn power_off(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(async move |_, cx: &mut AsyncApp| {
                let _ = login1_proxy.power_off(true).await;
            })
            .detach();
        }
    }

    pub fn reboot(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(async move |_, cx: &mut AsyncApp| {
                let _ = login1_proxy.reboot(true).await;
            })
            .detach();
        }
    }

    pub fn reboot_into_firmware_setup(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(async move |_, cx: &mut AsyncApp| {
                if login1_proxy.set_reboot_to_firmware_setup(true).await.is_ok() {
                    let _ = login1_proxy.reboot(true).await;
                }
            })
                .detach();
        }
    }

    pub fn suspend(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(async move |_, cx: &mut AsyncApp| {
                let _ = login1_proxy.suspend(true).await;
            })
            .detach();
        }
    }

    pub fn hibernate(&self, cx: &mut Context<Self>) {
        if let Some(login1_proxy) = self.login1_proxy.clone() {
            cx.spawn(async move |_, cx: &mut AsyncApp| {
                let _ = login1_proxy.hibernate(true).await;
            })
            .detach();
        }
    }
}
