#include "game_zone_options_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QCheckBox>
#include <QVBoxLayout>

GameZoneOptionsWidget::GameZoneOptionsWidget(QWidget *parent) : QWidget(parent)
{
    enableCommandZoneCheckBox = new QCheckBox(tr("Enable command zone (Commander)"), this);
    enableCompanionZoneCheckBox = new QCheckBox(tr("Enable companion zone"), this);
    enableBackgroundZoneCheckBox = new QCheckBox(tr("Enable background zone"), this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(enableCommandZoneCheckBox);
    layout->addWidget(enableCompanionZoneCheckBox);
    layout->addWidget(enableBackgroundZoneCheckBox);
    setLayout(layout);
}

bool GameZoneOptionsWidget::enableCommandZone() const
{
    return enableCommandZoneCheckBox->isChecked();
}

bool GameZoneOptionsWidget::enableCompanionZone() const
{
    return enableCompanionZoneCheckBox->isChecked();
}

bool GameZoneOptionsWidget::enableBackgroundZone() const
{
    return enableBackgroundZoneCheckBox->isChecked();
}

void GameZoneOptionsWidget::setCommandZoneEnabled(bool enabled)
{
    enableCommandZoneCheckBox->setChecked(enabled);
}

void GameZoneOptionsWidget::setCompanionZoneEnabled(bool enabled)
{
    enableCompanionZoneCheckBox->setChecked(enabled);
}

void GameZoneOptionsWidget::setBackgroundZoneEnabled(bool enabled)
{
    enableBackgroundZoneCheckBox->setChecked(enabled);
}

void GameZoneOptionsWidget::loadFromSettings()
{
    enableCommandZoneCheckBox->setChecked(SettingsCache::instance().getEnableCommandZone());
    enableCompanionZoneCheckBox->setChecked(SettingsCache::instance().getEnableCompanionZone());
    enableBackgroundZoneCheckBox->setChecked(SettingsCache::instance().getEnableBackgroundZone());
}

void GameZoneOptionsWidget::saveToSettings()
{
    SettingsCache::instance().setEnableCommandZone(enableCommandZoneCheckBox->isChecked());
    SettingsCache::instance().setEnableCompanionZone(enableCompanionZoneCheckBox->isChecked());
    SettingsCache::instance().setEnableBackgroundZone(enableBackgroundZoneCheckBox->isChecked());
}

void GameZoneOptionsWidget::resetToDefaults()
{
    enableCommandZoneCheckBox->setChecked(false);
    enableCompanionZoneCheckBox->setChecked(true);
    enableBackgroundZoneCheckBox->setChecked(true);
}

void GameZoneOptionsWidget::setReadOnly(bool readOnly)
{
    enableCommandZoneCheckBox->setEnabled(!readOnly);
    enableCompanionZoneCheckBox->setEnabled(!readOnly);
    enableBackgroundZoneCheckBox->setEnabled(!readOnly);
}
