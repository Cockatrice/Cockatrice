#ifndef COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_BUTTON_H
#define COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_BUTTON_H

#include <QString>

const QString visualDatabaseDisplayFilterButtonStyle = QString(R"(
    QPushButton {
        background-color: palette(button);
        color: palette(button-text);
        padding: 5px 10px;
        border-radius: 4px;
        border: 1px solid palette(dark);
    }
    QPushButton:checked {
        background-color: palette(highlight);
        color: palette(highlighted-text);
        border: 1px solid palette(shadow);
    }
)");

#endif // COCKATRICE_VISUAL_DATABASE_DISPLAY_FILTER_BUTTON_H
