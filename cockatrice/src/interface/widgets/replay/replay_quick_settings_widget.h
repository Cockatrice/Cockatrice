#ifndef COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H
#define COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H

#include "../../interface/widgets/quick_settings/settings_button_widget.h"

#include <QDoubleSpinBox>

class ReplayQuickSettingsWidget : public SettingsButtonWidget
{
    Q_OBJECT

public:
    explicit ReplayQuickSettingsWidget(QWidget *parent);

    void retranslateUi();

signals:
    void fastForwardSpeedChanged(qreal speed);

private:
    QLabel fastForwardSpeedLabel;
    QDoubleSpinBox fastForwardSpeedBox;

private slots:
    void actUpdateFastForwardSpeed(qreal value);
};

#endif // COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H
