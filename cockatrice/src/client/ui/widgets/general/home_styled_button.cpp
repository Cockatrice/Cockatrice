#include "home_styled_button.h"

HomeStyledButton::HomeStyledButton(const QString &text, QWidget *parent) : QPushButton(text, parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(R"(
    QPushButton {
        font-size: 16px;
        padding: 10px;
        color: black;
        border: 1px solid #0b5e2e;
        border-radius: 6px;
        background: qlineargradient(x1:0, y1:1, x2:0, y2:0,
                                    stop:0 #92de58, stop:1 #139740);
    }
    QPushButton:hover {
        background: qlineargradient(x1:0, y1:1, x2:0, y2:0,
                                    stop:0 #aaf57a, stop:1 #1bb24e);
    }
    QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                    stop:0 #139740, stop:1 #0a4d23);
    }
)");
}
