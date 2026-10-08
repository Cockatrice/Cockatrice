/**
 * @file settings_button_widget.h
 * @ingroup Widgets
 * @ingroup Settings
 */
//! \todo Document this file.

#ifndef SETTINGS_BUTTON_WIDGET_H
#define SETTINGS_BUTTON_WIDGET_H

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class QHBoxLayout;
class QPixmap;
class QToolButton;
class SettingsPopupWidget;

class SettingsButtonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsButtonWidget(QWidget *parent = nullptr);

    ~SettingsButtonWidget() override;

    void addSettingsWidget(QWidget *toAdd) const;
    void removeSettingsWidget(QWidget *toRemove) const;
    void setButtonIcon(QPixmap iconMap);
    void setButtonText(const QString &buttonText);
    void setCompact(bool compact);
    bool isCompact() const
    {
        return compact;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void togglePopup();
    void onPopupClosed() const;

private:
    QHBoxLayout *layout;
    QToolButton *button;
    QString buttonText;
    bool compact;

public:
    SettingsPopupWidget *popup;
};

#endif // SETTINGS_BUTTON_WIDGET_H
