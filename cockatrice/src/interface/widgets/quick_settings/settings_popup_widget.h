/**
 * @file settings_popup_widget.h
 * @ingroup Widgets
 * @ingroup Settings
 * @brief TODO: Document this.
 */

#ifndef SETTINGS_POPUP_WIDGET_H
#define SETTINGS_POPUP_WIDGET_H

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class SettingsPopupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPopupWidget(QWidget *parent = nullptr);
    void addSettingsWidget(QWidget *toAdd) const;
    void removeSettingsWidget(QWidget *toRemove) const;
    void adjustSizeToFitScreen();

signals:
    void aboutToClose();

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QVBoxLayout *layout;
    QVBoxLayout *containerLayout;
    QScrollArea *scrollArea = nullptr;
    QWidget *containerWidget;
};

#endif // SETTINGSPOPUP_H
