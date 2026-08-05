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
    return getValue("visualdeckstoragesortingorder", QString(), QString(), 0).toInt();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowFolders() const
{
    return getValue("visualdeckstorageshowfolders", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowTagFilter() const
{
    return getValue("visualdeckstorageshowtagfilter", QString(), QString(), true).toBool();
}

QStringList VisualDeckStorageSettings::getVisualDeckStorageDefaultTagsList() const
{
    return getValue("visualdeckstoragedefaulttagslist", QString(), QString(), QVariant::fromValue(defaultTags))
        .toStringList();
}

bool VisualDeckStorageSettings::getVisualDeckStorageSearchFolderNames() const
{
    return getValue("visualdeckstoragesearchfoldernames", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowColorIdentity() const
{
    return getValue("visualdeckstorageshowcoloridentity", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowBannerCardComboBox() const
{
    return getValue("visualdeckstorageshowbannercardcombobox", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageShowTagsOnDeckPreviews() const
{
    return getValue("visualdeckstorageshowtagsondeckpreviews", QString(), QString(), true).toBool();
}

int VisualDeckStorageSettings::getVisualDeckStorageCardSize() const
{
    return getValue("visualdeckstoragecardsize", QString(), QString(), 100).toInt();
}

bool VisualDeckStorageSettings::getVisualDeckStorageDrawUnusedColorIdentities() const
{
    return getValue("visualdeckstoragedrawunusedcoloridentities", QString(), QString(), true).toBool();
}

int VisualDeckStorageSettings::getVisualDeckStorageUnusedColorIdentitiesOpacity() const
{
    return getValue("visualdeckstorageunusedcoloridentitiesopacity", QString(), QString(), 15).toInt();
}

int VisualDeckStorageSettings::getVisualDeckStorageTooltipType() const
{
    return getValue("visualdeckstoragetooltiptype", QString(), QString(), 0).toInt();
}

bool VisualDeckStorageSettings::getVisualDeckStoragePromptForConversion() const
{
    return getValue("visualdeckstoragepromptforconversion", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageAlwaysConvert() const
{
    return getValue("visualdeckstoragealwaysconvert", QString(), QString(), false).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageInGame() const
{
    return getValue("visualdeckstorageingame", QString(), QString(), true).toBool();
}

bool VisualDeckStorageSettings::getVisualDeckStorageSelectionAnimation() const
{
    return getValue("visualdeckstorageselectionanimation", QString(), QString(), true).toBool();
}

int VisualDeckStorageSettings::getVisualDeckEditorCardSize() const
{
    return getValue("visualdeckeditorcardsize", QString(), QString(), 100).toInt();
}

int VisualDeckStorageSettings::getVisualDeckEditorSampleHandSize() const
{
    return getValue("visualdeckeditorsamplehandsize", QString(), QString(), 7).toInt();
}

int VisualDeckStorageSettings::getVisualDatabaseDisplayCardSize() const
{
    return getValue("visualdatabasedisplaycardsize", QString(), QString(), 100).toInt();
}

bool VisualDeckStorageSettings::getVisualDatabaseDisplayFilterToMostRecentSetsEnabled() const
{
    return getValue("visualdatabasedisplayfiltertomostrecentsetsenabled", QString(), QString(), false).toBool();
}

int VisualDeckStorageSettings::getVisualDatabaseDisplayFilterToMostRecentSetsAmount() const
{
    return getValue("visualdatabasedisplayfiltertomostrecentsetsamount", QString(), QString(), 10).toInt();
}

int VisualDeckStorageSettings::getEDHRecCardSize() const
{
    return getValue("edhreccardsize", QString(), QString(), 100).toInt();
}

int VisualDeckStorageSettings::getArchidektPreviewSize() const
{
    return getValue("archidektpreviewsize", QString(), QString(), 100).toInt();
}

int VisualDeckStorageSettings::getDefaultDeckEditorType() const
{
    return getValue("defaultDeckEditorType", QString(), QString(), 1).toInt();
}

void VisualDeckStorageSettings::setVisualDeckStorageSortingOrder(int _sortingOrder)
{
    setValue(_sortingOrder, "visualdeckstoragesortingorder");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowFolders(bool value)
{
    setValue(value, "visualdeckstorageshowfolders");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowTagFilter(bool _showTags)
{
    setValue(_showTags, "visualdeckstorageshowtagfilter");
    emit visualDeckStorageShowTagFilterChanged(_showTags);
}

void VisualDeckStorageSettings::setVisualDeckStorageDefaultTagsList(QStringList _defaultTagsList)
{
    setValue(QVariant::fromValue(_defaultTagsList), "visualdeckstoragedefaulttagslist");
    emit visualDeckStorageDefaultTagsListChanged();
}

void VisualDeckStorageSettings::setVisualDeckStorageSearchFolderNames(bool value)
{
    setValue(value, "visualdeckstoragesearchfoldernames");
}

void VisualDeckStorageSettings::setVisualDeckStorageShowColorIdentity(bool value)
{
    setValue(value, "visualdeckstorageshowcoloridentity");
    emit visualDeckStorageShowColorIdentityChanged(value);
}

void VisualDeckStorageSettings::setVisualDeckStorageShowBannerCardComboBox(bool _showBannerCardComboBox)
{
    setValue(_showBannerCardComboBox, "visualdeckstorageshowbannercardcombobox");
    emit visualDeckStorageShowBannerCardComboBoxChanged(_showBannerCardComboBox);
}

void VisualDeckStorageSettings::setVisualDeckStorageShowTagsOnDeckPreviews(bool _showTags)
{
    setValue(_showTags, "visualdeckstorageshowtagsondeckpreviews");
    emit visualDeckStorageShowTagsOnDeckPreviewsChanged(_showTags);
}

void VisualDeckStorageSettings::setVisualDeckStorageCardSize(int _cardSize)
{
    setValue(_cardSize, "visualdeckstoragecardsize");
    emit visualDeckStorageCardSizeChanged();
}

void VisualDeckStorageSettings::setVisualDeckStorageDrawUnusedColorIdentities(bool _draw)
{
    setValue(_draw, "visualdeckstoragedrawunusedcoloridentities");
    emit visualDeckStorageDrawUnusedColorIdentitiesChanged(_draw);
}

void VisualDeckStorageSettings::setVisualDeckStorageUnusedColorIdentitiesOpacity(int _opacity)
{
    setValue(_opacity, "visualdeckstorageunusedcoloridentitiesopacity");
    emit visualDeckStorageUnusedColorIdentitiesOpacityChanged(_opacity);
}

void VisualDeckStorageSettings::setVisualDeckStorageTooltipType(int value)
{
    setValue(value, "visualdeckstoragetooltiptype");
}

void VisualDeckStorageSettings::setVisualDeckStoragePromptForConversion(bool _prompt)
{
    setValue(_prompt, "visualdeckstoragepromptforconversion");
}

void VisualDeckStorageSettings::setVisualDeckStorageAlwaysConvert(bool _always)
{
    setValue(_always, "visualdeckstoragealwaysconvert");
}

void VisualDeckStorageSettings::setVisualDeckStorageInGame(bool enabled)
{
    setValue(enabled, "visualdeckstorageingame");
    emit visualDeckStorageInGameChanged(enabled);
}

void VisualDeckStorageSettings::setVisualDeckStorageSelectionAnimation(bool enabled)
{
    setValue(enabled, "visualdeckstorageselectionanimation");
    emit visualDeckStorageSelectionAnimationChanged(enabled);
}

void VisualDeckStorageSettings::setVisualDeckEditorCardSize(int _cardSize)
{
    setValue(_cardSize, "visualdeckeditorcardsize");
    emit visualDeckEditorCardSizeChanged();
}

void VisualDeckStorageSettings::setVisualDeckEditorSampleHandSize(int _amount)
{
    setValue(_amount, "visualdeckeditorsamplehandsize");
    emit visualDeckEditorSampleHandSizeAmountChanged(_amount);
}

void VisualDeckStorageSettings::setVisualDatabaseDisplayCardSize(int _cardSize)
{
    setValue(_cardSize, "visualdatabasedisplaycardsize");
    emit visualDatabaseDisplayCardSizeChanged();
}

void VisualDeckStorageSettings::setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(bool _enabled)
{
    setValue(_enabled, "visualdatabasedisplayfiltertomostrecentsetsenabled");
    emit visualDatabaseDisplayFilterToMostRecentSetsEnabledChanged(_enabled);
}

void VisualDeckStorageSettings::setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int _amount)
{
    setValue(_amount, "visualdatabasedisplayfiltertomostrecentsetsamount");
    emit visualDatabaseDisplayFilterToMostRecentSetsAmountChanged(_amount);
}

void VisualDeckStorageSettings::setEDHRecCardSize(int _edhrecCardSize)
{
    setValue(_edhrecCardSize, "edhreccardsize");
    emit edhRecCardSizeChanged();
}

void VisualDeckStorageSettings::setArchidektPreviewCardSize(int _archidektPreviewCardSize)
{
    setValue(_archidektPreviewCardSize, "archidektpreviewsize");
    emit archidektPreviewSizeChanged();
}

void VisualDeckStorageSettings::setDefaultDeckEditorType(int value)
{
    setValue(value, "defaultDeckEditorType");
}
