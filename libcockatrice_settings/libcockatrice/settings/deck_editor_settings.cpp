#include "deck_editor_settings.h"

DeckEditorSettings::DeckEditorSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "deck_editor.ini", "deckeditor", QString(), parent)
{
}

bool DeckEditorSettings::getOpenDeckInNewTab() const
{
    return getValue("openDeckInNewTab", QString(), QString(), false).toBool();
}

bool DeckEditorSettings::getBannerCardComboBoxVisible() const
{
    return getValue("bannerCardComboBoxVisible", QString(), QString(), true).toBool();
}

bool DeckEditorSettings::getTagsWidgetVisible() const
{
    return getValue("tagsWidgetVisible", QString(), QString(), true).toBool();
}

int DeckEditorSettings::getDefaultDeckEditorType() const
{
    return getValue("defaultDeckEditorType", QString(), QString(), 1).toInt();
}

int DeckEditorSettings::getVdeStartupTab() const
{
    return getValue("vdeStartupTab", QString(), QString(), VdeStartupTabContext).toInt();
}

void DeckEditorSettings::setOpenDeckInNewTab(bool _openDeckInNewTab)
{
    setValue(_openDeckInNewTab, "openDeckInNewTab");
}

void DeckEditorSettings::setBannerCardComboBoxVisible(bool _bannerCardComboBoxVisible)
{
    setValue(_bannerCardComboBoxVisible, "bannerCardComboBoxVisible");
    emit bannerCardComboBoxVisibleChanged(_bannerCardComboBoxVisible);
}

void DeckEditorSettings::setTagsWidgetVisible(bool _tagsWidgetVisible)
{
    setValue(_tagsWidgetVisible, "tagsWidgetVisible");
    emit tagsWidgetVisibleChanged(_tagsWidgetVisible);
}

void DeckEditorSettings::setDefaultDeckEditorType(int _defaultDeckEditorType)
{
    setValue(_defaultDeckEditorType, "defaultDeckEditorType");
}

void DeckEditorSettings::setVdeStartupTab(int _vdeStartupTab)
{
    setValue(_vdeStartupTab, "vdeStartupTab");
    emit vdeStartupTabChanged(_vdeStartupTab);
}

int DeckEditorSettings::getCommanderSpellbookIntegrationEnabled() const
{
    return getValue("commanderspellbookintegrationenabled", QString(), QString(),
                    commanderSpellbookIntegrationEnabledIndexUnprompted)
        .toInt();
}

bool DeckEditorSettings::getCommanderSpellbookIntegrationUseOfficialBracketNames() const
{
    return getValue("commanderspellbookintegrationuseofficialbracketnames", QString(), QString(), false).toBool();
}

void DeckEditorSettings::setCommanderSpellbookIntegrationEnabled(int _commanderSpellbookIntegrationEnabled)
{
    setValue(_commanderSpellbookIntegrationEnabled, "commanderspellbookintegrationenabled");
    emit commanderSpellbookIntegrationEnabledChanged(_commanderSpellbookIntegrationEnabled);
}

void DeckEditorSettings::setCommanderSpellbookIntegrationUseOfficialBracketNames(bool _useOfficialBracketNames)
{
    setValue(_useOfficialBracketNames, "commanderspellbookintegrationuseofficialbracketnames");
    emit commanderSpellbookIntegrationUseOfficialBracketNamesChanged(_useOfficialBracketNames);
}
