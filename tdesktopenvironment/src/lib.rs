use crate::system_power_state::SystemPowerState;
use crate::system_user_accounts::SystemUserAccounts;
use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::{App, AppContext, BorrowAppContext, Entity, Global};

pub mod components;
pub mod dbus_generated;
pub mod system_power_state;
pub mod system_user_accounts;

pub fn setup(cx: &mut App) {
    I18N_MANAGER.write().unwrap().load_source(tr_load!());

    let system_user_accounts = cx.new(|cx| SystemUserAccounts::new(cx));
    let system_power_state = cx.new(|cx| SystemPowerState::new(cx));

    cx.set_global(DesktopSystem {
        user_accounts: system_user_accounts,
        power_state: system_power_state,
    })
}

pub struct DesktopSystem {
    pub user_accounts: Entity<SystemUserAccounts>,
    pub power_state: Entity<SystemPowerState>,
}

impl Global for DesktopSystem {}
