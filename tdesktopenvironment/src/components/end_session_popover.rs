use crate::DesktopSystem;
use crate::components::command_link_button::command_link_button;
use crate::system_power_state::SystemPowerStateCapability;
use cntp_i18n::tr;
use contemporary::components::button::{Button, button};
use contemporary::components::constrainer::constrainer;
use contemporary::components::context_menu::{ContextMenuExt, ContextMenuItem};
use contemporary::components::grandstand::grandstand;
use contemporary::components::layer::layer;
use contemporary::components::pager::pager;
use contemporary::components::pager::slide_horizontal_animation::SlideHorizontalAnimation;
use contemporary::components::popover::popover;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::ThemeStorage;
use gpui::{App, Context, ElementId, IntoElement, ParentElement, Render, Styled, Window, div, px};
use std::rc::Rc;

pub enum EndSessionOption {
    PowerOff,
    Reboot,
    LogOut,
    Suspend,
    Lock,
    Hibernate,
}

impl EndSessionOption {
    pub fn command_link_button(&self, cx: &mut Context<EndSessionPopover>) -> Button {
        match self {
            EndSessionOption::PowerOff => button("power-off")
                .destructive()
                .child(command_link_button(
                    "system-shutdown".into(),
                    tr!("POWER_OPTIONS_POWER_OFF", "Power Off").into(),
                    tr!(
                        "POWER_OPTIONS_POWER_OFF_DESCRIPTION",
                        "Turns off your computer"
                    )
                    .into(),
                ))
                .on_click(cx.listener(|this, _, _, cx| {
                    this.hide(cx);

                    let desktop_system = cx.global::<DesktopSystem>();
                    desktop_system
                        .power_state
                        .clone()
                        .update(cx, |power_state, cx| {
                            power_state.power_off(cx);
                        });
                })),
            EndSessionOption::Reboot => {
                let desktop_system = cx.global::<DesktopSystem>();
                let power_state = desktop_system.power_state.read(cx);

                let perform_standard_reboot = Rc::new(cx.listener(|this, _, _, cx| {
                    this.hide(cx);

                    let desktop_system = cx.global::<DesktopSystem>();
                    desktop_system
                        .power_state
                        .clone()
                        .update(cx, |power_state, cx| {
                            power_state.reboot(cx);
                        });
                }));

                let perform_standard_reboot_clone = perform_standard_reboot.clone();
                let mut context_menu = vec![
                    ContextMenuItem::separator()
                        .label(tr!("POWER_OPTIONS_ADVANCED_REBOOT", "Advanced Reboot"))
                        .build(),
                    ContextMenuItem::menu_item()
                        .label(tr!("POWER_OPTIONS_REBOOT"))
                        .icon("system-reboot")
                        .on_triggered(move |event, window, cx| {
                            perform_standard_reboot_clone.clone()(&(), window, cx)
                        })
                        .build(),
                ];

                if matches!(
                    power_state.can_reboot_to_firmware_setup(),
                    SystemPowerStateCapability::Available
                        | SystemPowerStateCapability::AvailableWithAuthentication
                ) {
                    context_menu.push(
                        ContextMenuItem::menu_item()
                            .label(tr!(
                                "POWER_OPTIONS_REBOOT_TO_FIRMWARE_SETUP",
                                "Reboot to UEFI Setup"
                            ))
                            .icon("system-reboot")
                            .on_triggered(cx.listener(|this, _, _, cx| {
                                this.hide(cx);

                                let desktop_system = cx.global::<DesktopSystem>();
                                desktop_system
                                    .power_state
                                    .clone()
                                    .update(cx, |power_state, cx| {
                                        power_state.reboot_into_firmware_setup(cx);
                                    });
                            }))
                            .build(),
                    );
                }

                button("reboot")
                    .destructive()
                    .child(command_link_button(
                        "system-reboot".into(),
                        tr!("POWER_OPTIONS_REBOOT", "Reboot").into(),
                        tr!(
                            "POWER_OPTIONS_REBOOT_DESCRIPTION",
                            "Turns off your computer and turns it back on again"
                        )
                        .into(),
                    ))
                    .on_click(move |event, window, cx| perform_standard_reboot(&(), window, cx))
                    .with_context_menu(context_menu)
            }
            EndSessionOption::LogOut => button("log-out").child(command_link_button(
                "system-log-out".into(),
                tr!("POWER_OPTIONS_LOG_OUT", "Log Out").into(),
                tr!(
                    "POWER_OPTIONS_LOG_OUT_DESCRIPTION",
                    "Ends your session but keeps your computer on for others"
                )
                .into(),
            )),
            EndSessionOption::Suspend => button("suspend")
                .child(command_link_button(
                    "system-suspend".into(),
                    tr!("POWER_OPTIONS_SUSPEND", "Suspend").into(),
                    tr!(
                        "POWER_OPTIONS_SUSPEND_DESCRIPTION",
                        "Puts your computer in a low power state so it reopens quickly"
                    )
                    .into(),
                ))
                .on_click(cx.listener(|this, _, _, cx| {
                    this.hide(cx);

                    let desktop_system = cx.global::<DesktopSystem>();
                    desktop_system
                        .power_state
                        .clone()
                        .update(cx, |power_state, cx| {
                            power_state.suspend(cx);
                        });
                })),
            EndSessionOption::Lock => button("lock").child(command_link_button(
                "system-lock".into(),
                tr!("POWER_OPTIONS_LOCK", "Lock").into(),
                tr!(
                    "POWER_OPTIONS_LOCK_DESCRIPTION",
                    "Locks your computer with your password"
                )
                .into(),
            )),
            EndSessionOption::Hibernate => button("hibernate").child(command_link_button(
                "system-hibernate".into(),
                tr!("POWER_OPTIONS_HIBERNATE", "Hibernate").into(),
                tr!(
                    "POWER_OPTIONS_HIBERNATE_DESCRIPTION",
                    "Powers off your computer, but restores the session when powered back on"
                )
                .into(),
            )),
        }
    }

    pub fn is_possible(&self, cx: &mut App) -> bool {
        let desktop_system = cx.global::<DesktopSystem>();
        let power_state = desktop_system.power_state.read(cx);

        match self {
            EndSessionOption::PowerOff => {
                matches!(
                    power_state.can_power_off(),
                    SystemPowerStateCapability::Available
                        | SystemPowerStateCapability::AvailableWithAuthentication
                )
            }
            EndSessionOption::Reboot => {
                matches!(
                    power_state.can_reboot(),
                    SystemPowerStateCapability::Available
                        | SystemPowerStateCapability::AvailableWithAuthentication
                )
            }
            EndSessionOption::LogOut => true,
            EndSessionOption::Suspend => {
                matches!(
                    power_state.can_suspend(),
                    SystemPowerStateCapability::Available
                        | SystemPowerStateCapability::AvailableWithAuthentication
                )
            }
            EndSessionOption::Lock => true,
            EndSessionOption::Hibernate => {
                matches!(
                    power_state.can_hibernate(),
                    SystemPowerStateCapability::Available
                        | SystemPowerStateCapability::AvailableWithAuthentication
                )
            }
        }
    }
}

pub struct EndSessionPopover {
    id: ElementId,
    end_session_options: Vec<EndSessionOption>,
    visible: bool,
}

impl EndSessionPopover {
    pub fn new(
        id: impl Into<ElementId>,
        end_session_options: Vec<EndSessionOption>,
        cx: &mut Context<Self>,
    ) -> Self {
        Self {
            id: id.into(),
            end_session_options,
            visible: false,
        }
    }

    pub fn show(&mut self, cx: &mut Context<Self>) {
        self.visible = true;
        cx.notify()
    }

    pub fn hide(&mut self, cx: &mut Context<Self>) {
        self.visible = false;
        cx.notify()
    }
}

impl Render for EndSessionPopover {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.theme();

        popover(self.id.clone())
            .visible(self.visible)
            .anchor_bottom()
            .size(800.)
            .content(
                pager("end-session-pager", 0)
                    .animation(SlideHorizontalAnimation::new())
                    .size_full()
                    .page(
                        div()
                            .flex()
                            .flex_col()
                            .gap(px(9.))
                            .child(
                                grandstand("end-session-grandstand")
                                    .text(tr!("POWER_OPTIONS", "Power Options"))
                                    .on_back_click(cx.listener(move |this, _, _, cx| {
                                        this.hide(cx);
                                    })),
                            )
                            .child(
                                constrainer("key-import-constrainer").child(
                                    layer()
                                        .flex()
                                        .flex_col()
                                        .p(px(8.))
                                        .w_full()
                                        .child(tr!(
                                            "POWER_OPTIONS_PROMPT",
                                            "What do you want to do now?"
                                        ))
                                        .child(
                                            div()
                                                .flex()
                                                .flex_col()
                                                .rounded(theme.border_radius)
                                                .bg(theme.button_background)
                                                .child(self.end_session_options.iter().fold(
                                                    div().flex().flex_col(),
                                                    |david, option| {
                                                        if option.is_possible(cx) {
                                                            david.child(
                                                                option.command_link_button(cx),
                                                            )
                                                        } else {
                                                            david
                                                        }
                                                    },
                                                )),
                                        ),
                                ),
                            )
                            .into_any_element(),
                    )
                    .page(
                        div()
                            .size_full()
                            .flex()
                            .items_center()
                            .justify_center()
                            .child(spinner())
                            .into_any_element(),
                    ),
            )
    }
}
