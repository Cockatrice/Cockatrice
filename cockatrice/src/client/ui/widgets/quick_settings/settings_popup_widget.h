#ifndef SETTINGS_POPUP_WIDGET_H
#define SETTINGS_POPUP_WIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class SettingsPopupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPopupWidget(QWidget *parent = nullptr);
    void addSettingsWidget(QWidget *toAdd) const;

signals:
    void aboutToClose();

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    QVBoxLayout *layout;
};

#endif // SETTINGSPOPUP_H
