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
