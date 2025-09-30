/**
 * @file settings_button_widget.h
 * @ingroup Widgets
 * @ingroup Settings
 * @brief TODO: Document this.
 */

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
    void removeSettingsWidget(QWidget *toRemove) const;
    void setButtonIcon(QPixmap iconMap);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void togglePopup();
    void onPopupClosed() const;

private:
    QHBoxLayout *layout;
    QToolButton *button;

public:
    SettingsPopupWidget *popup;
};

#endif // SETTINGS_BUTTON_WIDGET_H
