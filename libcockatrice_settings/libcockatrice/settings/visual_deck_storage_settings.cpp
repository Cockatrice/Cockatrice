#include "visual_deck_storage_settings.h"

namespace
{
QStringList defaultTags = {
    // Strategies
    "🏃️ Aggro",
    "🧙‍️ Control",
    "⚔️ Midrange",
    "🌀 Combo",
    "🪓 Mill",
    "🔒 Stax",
    "🗺️ Landfall",
    "🛡️ Pillowfort",
    "🌱 Ramp",
    "⚡ Storm",
    "💀 Aristocrats",
    "☠️ Reanimator",
    "👹 Sacrifice",
    "🔥 Burn",
    "🌟 Lifegain",
    "🔮 Spellslinger",
    "👥 Tokens",
    "🎭 Blink",
    "⏳ Time Manipulation",
    "🌍 Domain",
    "💫 Proliferate",
    "📜 Saga",
    "🎲 Chaos",
    "🪄 Auras",
    "🔫 Pingers",

    // Themes
    "👑 Monarch",
    "🚀 Vehicles",
    "💉 Infect",
    "🩸 Madness",
    "🌀 Morph",

    // Card Types
    "⚔️ Creature",
    "💎 Artifact",
    "🌔 Enchantment",
    "📖 Sorcery",
    "⚡ Instant",
    "🌌 Planeswalker",
    "🌏 Land",
    "🪄 Aura",

    // Kindred Types
    "🐉 Kindred",
    "🧙 Humans",
    "⚔️ Soldiers",
    "🛡️ Knights",
    "🎻 Bards",
    "🧝 Elves",
    "🌲 Dryads",
    "😇 Angels",
    "🎩 Wizards",
    "🧛 Vampires",
    "🦴 Skeletons",
    "💀 Zombies",
    "👹 Demons",
    "👾 Eldrazi",
    "🐉 Dragons",
    "🐠 Merfolk",
    "🦁 Cats",
    "🐺 Wolves",
    "🐺 Werewolves",
    "🦇 Bats",
    "🐀 Rats",
    "🦅 Birds",
    "🦗 Insects",
    "🍄 Fungus",
    "🐚 Sea Creatures",
    "🐗 Boars",
    "🦊 Foxes",
    "🦄 Unicorns",
    "🐘 Elephants",
    "🐻 Bears",
    "🦏 Rhinos",
    "🦂 Scorpions",

};
}

VisualDeckStorageSettings::VisualDeckStorageSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "visual_deck_storage.ini", "interface", QString(), parent)
{
}

int VisualDeckStorageSettings::getVisualDeckStorageSortingOrder() const
{
    return getValue("sortingOrder", "interface", "visualDeckStorage", 0).toInt();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowFolders() const
{
    return getValue("showFolders", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowTagFilter() const
{
    return getValue("showTagFilter", "interface", "visualDeckStorage", true).toBool();
}

QStringList VisualDeckStorageSettings::getVisualDeckStorageDefaultTagsList() const
{
    return getValue("defaultTagsList", "interface", "visualDeckStorage", QVariant::fromValue(defaultTags))
        .toStringList();
}

bool VisualDeckStorageSettings::getVisualDeckStorageSearchFolderNames() const
{
    return getValue("searchFolderNames", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowColorIdentity() const
{
    return getValue("showColorIdentity", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowBannerCardComboBox() const
{
    return getValue("showBannerCardComboBox", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowTagsOnDeckPreviews() const
{
    return getValue("showTagsOnDeckPreviews", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowUploadTime() const
{
    return getValue("showUploadTime", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageDrawUnusedColorIdentities() const
{
    return getValue("drawUnusedColorIdentities", "interface", "visualDeckStorage", true).toBool();
}

int VisualDeckStorageSettings::getVisualDeckStorageUnusedColorIdentitiesOpacity() const
{
    return getValue("unusedColorIdentitiesOpacity", "interface", "visualDeckStorage", 15).toInt();
}

int VisualDeckStorageSettings::getVisualDeckStorageTooltipType() const
{
    return getValue("tooltipType", "interface", "visualDeckStorage", 0).toInt();
}

bool VisualDeckStorageSettings::getVisualDeckStoragePromptForConversion() const
{
    return getValue("promptForConversion", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageAlwaysConvert() const
{
    return getValue("alwaysConvert", "interface", "visualDeckStorage", false).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageInGame() const
{
    return getValue("inGame", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageSelectionAnimation() const
{
    return getValue("selectionAnimation", "interface", "visualDeckStorage", true).toBool();
}

bool VisualDeckStorageSettings::getVisualDatabaseDisplayFilterToMostRecentSetsEnabled() const
{
    return getValue("filterToMostRecentSetsEnabled", "interface", "visualDatabaseDisplay", false).toBool();
}

int VisualDeckStorageSettings::getVisualDatabaseDisplayFilterToMostRecentSetsAmount() const
{
    return getValue("filterToMostRecentSetsAmount", "interface", "visualDatabaseDisplay", 10).toInt();
}

void VisualDeckStorageSettings::setVisualDeckStorageSortingOrder(int _sortingOrder)
{
    setValue(_sortingOrder, "sortingOrder", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowFolders(bool value)
{
    setValue(value, "showFolders", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowTagFilter(bool _showTags)
{
    setValue(_showTags, "showTagFilter", "interface", "visualDeckStorage");
    emit visualDeckStorageShowTagFilterChanged(_showTags);
}

void VisualDeckStorageSettings::setVisualDeckStorageDefaultTagsList(QStringList _defaultTagsList)
{
    setValue(QVariant::fromValue(_defaultTagsList), "defaultTagsList", "interface", "visualDeckStorage");
    emit visualDeckStorageDefaultTagsListChanged();
}

void VisualDeckStorageSettings::setVisualDeckStorageSearchFolderNames(bool value)
{
    setValue(value, "searchFolderNames", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowColorIdentity(bool value)
{
    setValue(value, "showColorIdentity", "interface", "visualDeckStorage");
    emit visualDeckStorageShowColorIdentityChanged(value);
}

void VisualDeckStorageSettings::setVisualDeckStorageShowBannerCardComboBox(bool _showBannerCardComboBox)
{
    setValue(_showBannerCardComboBox, "showBannerCardComboBox", "interface", "visualDeckStorage");
    emit visualDeckStorageShowBannerCardComboBoxChanged(_showBannerCardComboBox);
}

void VisualDeckStorageSettings::setVisualDeckStorageShowTagsOnDeckPreviews(bool _showTags)
{
    setValue(_showTags, "showTagsOnDeckPreviews", "interface", "visualDeckStorage");
    emit visualDeckStorageShowTagsOnDeckPreviewsChanged(_showTags);
}

void VisualDeckStorageSettings::setVisualDeckStorageShowUploadTime(bool value)
{
    setValue(value, "showUploadTime", "interface", "visualDeckStorage");
    emit visualDeckStorageShowUploadTimeChanged(value);
}

void VisualDeckStorageSettings::setVisualDeckStorageDrawUnusedColorIdentities(bool _draw)
{
    setValue(_draw, "drawUnusedColorIdentities", "interface", "visualDeckStorage");
    emit visualDeckStorageDrawUnusedColorIdentitiesChanged(_draw);
}

void VisualDeckStorageSettings::setVisualDeckStorageUnusedColorIdentitiesOpacity(int _opacity)
{
    setValue(_opacity, "unusedColorIdentitiesOpacity", "interface", "visualDeckStorage");
    emit visualDeckStorageUnusedColorIdentitiesOpacityChanged(_opacity);
}

void VisualDeckStorageSettings::setVisualDeckStorageTooltipType(int value)
{
    setValue(value, "tooltipType", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStoragePromptForConversion(bool _prompt)
{
    setValue(_prompt, "promptForConversion", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStorageAlwaysConvert(bool _always)
{
    setValue(_always, "alwaysConvert", "interface", "visualDeckStorage");
}

void VisualDeckStorageSettings::setVisualDeckStorageInGame(bool enabled)
{
    setValue(enabled, "inGame", "interface", "visualDeckStorage");
    emit visualDeckStorageInGameChanged(enabled);
}

void VisualDeckStorageSettings::setVisualDeckStorageSelectionAnimation(bool enabled)
{
    setValue(enabled, "selectionAnimation", "interface", "visualDeckStorage");
    emit visualDeckStorageSelectionAnimationChanged(enabled);
}

void VisualDeckStorageSettings::setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(bool _enabled)
{
    setValue(_enabled, "filterToMostRecentSetsEnabled", "interface", "visualDatabaseDisplay");
    emit visualDatabaseDisplayFilterToMostRecentSetsEnabledChanged(_enabled);
}

void VisualDeckStorageSettings::setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int _amount)
{
    setValue(_amount, "filterToMostRecentSetsAmount", "interface", "visualDatabaseDisplay");
    emit visualDatabaseDisplayFilterToMostRecentSetsAmountChanged(_amount);
}
