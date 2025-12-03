use crate::dbus_generated::accounts::accounts::{AccountsProxy, UserAddedArgs, UserDeletedArgs};
use crate::dbus_generated::accounts::user::UserProxy;
use crate::dbus_generated::register_dbus_listener;
use gpui::{App, AppContext, AsyncApp, Context, Entity, Global, WeakEntity};
use std::rc::Rc;
use std::time::Duration;
use tracing::error;
use zbus::Connection;
use zbus::export::ordered_stream::OrderedStreamExt;
use zbus::proxy::CacheProperties;
use zbus::zvariant::{ObjectPath, OwnedObjectPath};

pub struct SystemUserAccounts {
    accounts_proxy: Option<Rc<AccountsProxy<'static>>>,
    cached_users: Vec<OwnedObjectPath>,
}

impl SystemUserAccounts {
    pub fn new(cx: &mut Context<Self>) -> Self {
        cx.spawn(
            async move |weak_entity: WeakEntity<Self>, cx: &mut AsyncApp| {
                if weak_entity
                    .update(cx, |this, cx| {
                        this.accounts_proxy = None;
                        cx.notify();
                    })
                    .is_err()
                {
                    return;
                }

                let Ok(connection) = Connection::system().await else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let Ok(accounts_proxy) = AccountsProxy::builder(&connection)
                    .cache_properties(CacheProperties::Yes)
                    .build()
                    .await
                else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let accounts_proxy = Rc::new(accounts_proxy);

                let accounts_proxy_clone = accounts_proxy.clone();
                let weak_entity_clone = weak_entity.clone();
                register_dbus_listener!(
                    cx,
                    accounts_proxy_clone.receive_user_added(),
                    move |args: UserAddedArgs, cx: &mut AsyncApp| {
                        if weak_entity_clone
                            .update(cx, |this, cx| {
                                this.cached_users.push(args.user().to_owned().into());
                            })
                            .is_err()
                        {
                            return;
                        }
                    }
                );

                let accounts_proxy_clone = accounts_proxy.clone();
                let weak_entity_clone = weak_entity.clone();
                register_dbus_listener!(
                    cx,
                    accounts_proxy_clone.receive_user_deleted(),
                    move |args: UserDeletedArgs, cx: &mut AsyncApp| {
                        if weak_entity_clone
                            .update(cx, |this, cx| {
                                this.cached_users
                                    .retain(|user| user.to_string() != args.user().to_string());
                            })
                            .is_err()
                        {
                            return;
                        }
                    }
                );

                if let Ok(cached_users) = accounts_proxy.list_cached_users().await {
                    if weak_entity
                        .update(cx, |this, cx| {
                            this.cached_users = cached_users;
                        })
                        .is_err()
                    {
                        return;
                    }
                }

                if weak_entity
                    .update(cx, |this, cx| {
                        this.accounts_proxy = Some(accounts_proxy);
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
            accounts_proxy: None,
            cached_users: Vec::new(),
        }
    }

    pub fn cached_users(&self, cx: &mut App) -> Vec<Entity<UserAccount>> {
        self.cached_users
            .iter()
            .map(|user| self.user_by_object_path(user.into(), cx))
            .collect()
    }

    fn user_by_object_path(&self, path: ObjectPath, cx: &mut App) -> Entity<UserAccount> {
        let entity = cx.new(|cx| {
            let mut user_account = UserAccount::new(cx);
            user_account.set_object_path(path, cx);
            user_account
        });

        entity
    }

    pub fn find_user_by_id(&self, id: i64, cx: &mut App) -> Entity<UserAccount> {
        let entity = cx.new(UserAccount::new);

        let accounts_proxy = self.accounts_proxy.clone();
        let entity_clone = entity.clone();
        if let Some(accounts_proxy) = accounts_proxy {
            cx.spawn(async move |cx: &mut AsyncApp| {
                match accounts_proxy.find_user_by_id(id).await {
                    Ok(object_path) => {
                        let _ = entity_clone.update(cx, |entity, cx| {
                            entity.set_object_path(object_path.into_inner(), cx);
                        });
                    }
                    Err(e) => {
                        error!("Failed to find user by id {id}: {e}");
                    }
                };
            })
            .detach();
        }

        entity
    }
}

impl Global for SystemUserAccounts {}

pub struct UserAccount {
    user_proxy: Option<Rc<UserProxy<'static>>>,
    object_path: Option<OwnedObjectPath>,
}

impl UserAccount {
    fn new(cx: &mut Context<Self>) -> Self {
        Self {
            user_proxy: None,
            object_path: None,
        }
    }

    fn set_object_path(&mut self, path: ObjectPath, cx: &mut Context<Self>) {
        if self.object_path.is_some() {
            panic!("Object path already set");
        }

        let object_path = OwnedObjectPath::from(path);
        self.object_path = Some(object_path.clone());
        cx.spawn(
            async move |weak_this: WeakEntity<Self>, cx: &mut AsyncApp| {
                let Ok(connection) = Connection::system().await else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let Ok(user_proxy) = UserProxy::builder(&connection)
                    .cache_properties(CacheProperties::Yes)
                    .path(object_path.clone())
                    .unwrap()
                    .build()
                    .await
                else {
                    cx.background_executor().timer(Duration::from_secs(1)).await;
                    return;
                };

                let user_proxy = Rc::new(user_proxy);

                let user_proxy_clone = user_proxy.clone();
                let weak_this_clone = weak_this.clone();

                cx.spawn(async move |cx: &mut AsyncApp| {
                    let stream = user_proxy_clone.receive_changed().await;
                    match stream {
                        Ok(mut stream) => {
                            while stream.next().await.is_some() {
                                if weak_this_clone.update(cx, |this, cx| cx.notify()).is_err() {
                                    return;
                                }
                            }
                        }
                        Err(e) => {
                            error!("Failed to listen to signal: {e}");
                        }
                    }
                })
                .detach();

                if weak_this
                    .update(cx, |this, cx| {
                        this.user_proxy = Some(user_proxy);
                    })
                    .is_err()
                {
                    return;
                }
            },
        )
        .detach();
    }

    pub fn home_directory(&self) -> Option<String> {
        self.user_proxy
            .as_ref()
            .and_then(|user_proxy| user_proxy.cached_home_directory().ok().flatten())
    }

    pub fn uid(&self) -> Option<u64> {
        self.user_proxy
            .as_ref()
            .and_then(|user_proxy| user_proxy.cached_uid().ok().flatten())
    }

    pub fn user_name(&self) -> Option<String> {
        self.user_proxy
            .as_ref()
            .and_then(|user_proxy| user_proxy.cached_user_name().ok().flatten())
    }

    pub fn real_name(&self) -> Option<String> {
        self.user_proxy
            .as_ref()
            .and_then(|user_proxy| user_proxy.cached_real_name().ok().flatten())
    }
}
