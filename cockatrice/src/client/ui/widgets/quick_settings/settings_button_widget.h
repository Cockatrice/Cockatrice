#ifndef SETTINGS_BUTTON_WIDGET_H
#define SETTINGS_BUTTON_WIDGET_H

#include "settings_popup_widget.h"

#include <QToolButton>
#include <QWidget>

class SettingsButtonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsButtonWidget(QWidget *parent = nullptr);
    void addSettingsWidget(QWidget *toAdd) const;

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void togglePopup();
    void onPopupClosed() const;

private:
    QHBoxLayout *layout;
    QToolButton *button;
    SettingsPopupWidget *popup;
};

#endif // SETTINGS_BUTTON_WIDGET_H
