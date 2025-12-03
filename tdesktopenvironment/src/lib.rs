use crate::system_user_accounts::SystemUserAccounts;
use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::{App, AppContext, BorrowAppContext, Entity, Global};

pub mod dbus_generated;
pub mod system_user_accounts;

pub fn setup(cx: &mut App) {
    I18N_MANAGER.write().unwrap().load_source(tr_load!());
    
    let system_user_accounts = cx.new(|cx| SystemUserAccounts::new(cx));

    cx.set_global(DesktopSystem {
        user_accounts: system_user_accounts
    })
}

pub struct DesktopSystem {
    pub user_accounts: Entity<SystemUserAccounts>,
}

impl Global for DesktopSystem {}