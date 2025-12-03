use contemporary::components::icon::icon;
use gpui::{App, FontWeight, IntoElement, ParentElement, RenderOnce, SharedString, Styled, Window, div, px};

#[derive(IntoElement)]
pub struct CommandLinkButton {
    icon: SharedString,
    title: SharedString,
    description: SharedString,
    icon_size: f32
}

pub fn command_link_button(
    icon: SharedString,
    title: SharedString,
    description: SharedString,
) -> CommandLinkButton {
    CommandLinkButton {
        icon,
        title,
        description,
        icon_size: 20.
    }
}

impl CommandLinkButton {
    pub fn icon_size(mut self, icon_size: f32) -> Self {
        self.icon_size = icon_size;
        self
    }
}

impl RenderOnce for CommandLinkButton {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        div().flex().w_full().gap(px(4.)).child(icon(self.icon).size(self.icon_size)).child(
            div()
                .flex()
                .flex_col()
                .child(div().font_weight(FontWeight::BOLD).child(self.title))
                .child(self.description),
        )
    }
}
