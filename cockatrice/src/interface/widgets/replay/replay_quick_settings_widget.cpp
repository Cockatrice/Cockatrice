#include "replay_quick_settings_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QGridLayout>
#include <QLabel>
#include <QWidget>

ReplayQuickSettingsWidget::ReplayQuickSettingsWidget(QWidget *parent) : SettingsButtonWidget(parent)
{
    // fast forward speed
    fastForwardSpeedBox.setMinimum(1);
    fastForwardSpeedBox.setMaximum(99.9);
    fastForwardSpeedBox.setDecimals(1);
    fastForwardSpeedBox.setValue(SettingsCache::instance().getFastForwardSpeed());
    connect(&fastForwardSpeedBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ReplayQuickSettingsWidget::actUpdateFastForwardSpeed);

    // putting it all together
    auto *widget = new QWidget;
    auto *grid = new QGridLayout(widget);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->addWidget(&fastForwardSpeedLabel, 0, 0, 1, 1);
    grid->addWidget(&fastForwardSpeedBox, 0, 1, 1, 1);

    this->addSettingsWidget(widget);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &ReplayQuickSettingsWidget::retranslateUi);
    retranslateUi();
}

void ReplayQuickSettingsWidget::retranslateUi()
{
    fastForwardSpeedLabel.setText(tr("Fast forward speed:"));
    fastForwardSpeedBox.setSuffix("x");
}

void ReplayQuickSettingsWidget::actUpdateFastForwardSpeed(qreal value)
{
    SettingsCache::instance().setFastForwardSpeed(value);
    emit fastForwardSpeedChanged(value);
}
