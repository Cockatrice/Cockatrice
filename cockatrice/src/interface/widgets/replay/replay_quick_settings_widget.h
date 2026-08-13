#ifndef COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H
#define COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H

#include "../../interface/widgets/quick_settings/settings_button_widget.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <libcockatrice/utility/macros.h>

class ReplayQuickSettingsWidget : public SettingsButtonWidget
{
    Q_OBJECT

public:
    explicit ReplayQuickSettingsWidget(QWidget *parent);

    void retranslateUi();

signals:
    void fastForwardSpeedChanged(qreal speed);
    void skipEmptySectionsChanged(bool skip);

private:
    QLabel fastForwardSpeedLabel;
    QDoubleSpinBox fastForwardSpeedBox;

    QCheckBox skipEmptyCheckBox;

private slots:
    void actUpdateFastForwardSpeed(qreal value);
    void actUpdateSkipEmptySections(QT_STATE_CHANGED_T value);
};

#endif // COCKATRICE_REPLAY_QUICK_SETTINGS_WIDGET_H
