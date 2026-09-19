#include "replay_quick_settings_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/personal_settings.h>

ReplayQuickSettingsWidget::ReplayQuickSettingsWidget(QWidget *parent) : SettingsButtonWidget(parent)
{
    // fast forward speed
    fastForwardSpeedBox.setMinimum(1);
    fastForwardSpeedBox.setMaximum(99.9);
    fastForwardSpeedBox.setDecimals(1);
    fastForwardSpeedBox.setValue(SettingsCache::instance().userInterface().getFastForwardSpeed());
    connect(&fastForwardSpeedBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ReplayQuickSettingsWidget::actUpdateFastForwardSpeed);

    skipEmptyCheckBox.setChecked(SettingsCache::instance().userInterface().getSkipEmptySections());
    connect(&skipEmptyCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &ReplayQuickSettingsWidget::actUpdateSkipEmptySections);

    // putting it all together
    auto *widget = new QWidget;
    auto *grid = new QGridLayout(widget);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->addWidget(&fastForwardSpeedLabel, 0, 0, 1, 1);
    grid->addWidget(&fastForwardSpeedBox, 0, 1, 1, 1);
    grid->addWidget(&skipEmptyCheckBox, 1, 0, 1, 2);

    this->addSettingsWidget(widget);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &ReplayQuickSettingsWidget::retranslateUi);
    retranslateUi();
}

void ReplayQuickSettingsWidget::retranslateUi()
{
    fastForwardSpeedLabel.setText(tr("Fast forward speed:"));
    fastForwardSpeedBox.setSuffix("x");

    skipEmptyCheckBox.setText(tr("Skip empty sections"));
}

void ReplayQuickSettingsWidget::actUpdateFastForwardSpeed(qreal value)
{
    SettingsCache::instance().userInterface().setFastForwardSpeed(value);
    emit fastForwardSpeedChanged(value);
}

void ReplayQuickSettingsWidget::actUpdateSkipEmptySections(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().userInterface().setSkipEmptySections(value);
    emit skipEmptySectionsChanged(value);
}
