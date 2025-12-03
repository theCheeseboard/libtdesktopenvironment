use crate::dbus_generated::accounts::accounts::UserAddedArgs;

pub mod accounts;

macro_rules! register_dbus_listener {
    ($cx: ident, $stream: expr, $callback: expr) => {
        let ck = $callback;
        $cx.spawn(async move |cx: &mut AsyncApp| {
            let stream = $stream.await;
            match stream {
                Ok(mut stream) => {
                    while let Some(user) = stream.next().await {
                        match user.args() {
                            Ok(args) => {
                                ck(args, cx);
                            },
                            Err(e) => {
                                error!("Received signal, but could not decode args: {e}");
                            }
                        }
                    }
                }
                Err(e) => {
                    error!("Failed to listen to signal: {e}");
                    return;
                }
            }
        })
        .detach();
    };
}

pub(crate) use register_dbus_listener;