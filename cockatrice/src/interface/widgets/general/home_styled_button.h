/**
 * @file home_styled_button.h
 * @ingroup Widgets
 * @brief TODO: Document this.
 */

#ifndef HOME_STYLED_BUTTON_H
#define HOME_STYLED_BUTTON_H
#include <QPushButton>

class HomeStyledButton : public QPushButton
{
    Q_OBJECT
public:
    HomeStyledButton(const QString &text, QPair<QColor, QColor> gradientColors, QWidget *parent = nullptr);
    void updateStylesheet(const QPair<QColor, QColor> &colors);
    QString generateButtonStylesheet(const QPair<QColor, QColor> &colors);
public slots:
    void paintEvent(QPaintEvent *event) override;

private:
    QPair<QColor, QColor> gradientColors;
};

#endif // HOME_STYLED_BUTTON_H
