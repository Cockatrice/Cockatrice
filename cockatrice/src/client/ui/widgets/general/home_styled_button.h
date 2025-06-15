//
// Created by ascor on 6/15/25.
//

#ifndef HOME_STYLED_BUTTON_H
#define HOME_STYLED_BUTTON_H
#include <QPushButton>

class HomeStyledButton : public QPushButton
{
    Q_OBJECT
public:
    HomeStyledButton(const QString &text, QWidget *parent = nullptr);
};

#endif // HOME_STYLED_BUTTON_H
