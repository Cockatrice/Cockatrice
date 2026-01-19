/**
 * @file cache_settings.h
 * @ingroup Settings
 * @brief TODO: Document this.
 */

#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include "shortcuts_settings.h"

#include <QDate>
#include <QLoggingCategory>
#include <QSize>
#include <QStringList>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>
#include <libcockatrice/settings/card_database_settings.h>
#include <libcockatrice/settings/card_override_settings.h>
#include <libcockatrice/settings/debug_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/game_filters_settings.h>
#include <libcockatrice/settings/layouts_settings.h>
#include <libcockatrice/settings/message_settings.h>
#include <libcockatrice/settings/recents_settings.h>
#include <libcockatrice/settings/servers_settings.h>
#include <libcockatrice/utility/macros.h>

inline Q_LOGGING_CATEGORY(SettingsCacheLog, "settings_cache");

class ReleaseChannel;

// In MB (Increments of 64)
#define PIXMAPCACHE_SIZE_DEFAULT 2048
#define PIXMAPCACHE_SIZE_MIN 64
#define PIXMAPCACHE_SIZE_MAX 4096

// In MB
constexpr int NETWORK_CACHE_SIZE_DEFAULT = 1024 * 4; // 4 GB
constexpr int NETWORK_CACHE_SIZE_MIN = 1;            // 1 MB
constexpr int NETWORK_CACHE_SIZE_MAX = 1024 * 1024;  // 1 TB

// In Days
#define NETWORK_REDIRECT_CACHE_TTL_DEFAULT 30
#define NETWORK_REDIRECT_CACHE_TTL_MIN 1
#define NETWORK_REDIRECT_CACHE_TTL_MAX 90

#define DEFAULT_LANG_NAME "English"
#define CLIENT_INFO_NOT_SET "notset"

#define DEFAULT_FONT_SIZE 12

inline QStringList defaultTags = {
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

class QSettings;
class CardCounterSettings;

class SettingsCache : public ICardDatabasePathProvider, public INetworkSettingsProvider
{
    Q_OBJECT

signals:
    void langChanged();
    void picsPathChanged();
    void themeChanged();
    void homeTabBackgroundSourceChanged();
    void homeTabBackgroundShuffleFrequencyChanged();
    void homeTabDisplayCardNameChanged();
    void picDownloadChanged();
    void showStatusBarChanged(bool state);
    void showGameSelectorFilterToolbarChanged(bool state);
    void displayCardNamesChanged();
    void overrideAllCardArtWithPersonalPreferenceChanged(bool _overrideAllCardArtWithPersonalPreference);
    void bumpSetsWithCardsInDeckToTopChanged();
    void printingSelectorSortOrderChanged();
    void printingSelectorCardSizeChanged();
    void includeRebalancedCardsChanged(bool _includeRebalancedCards);
    void printingSelectorNavigationButtonsVisibleChanged();
    void deckEditorBannerCardComboBoxVisibleChanged(bool _visible);
    void deckEditorTagsWidgetVisibleChanged(bool _visible);
    void visualDeckStorageShowTagFilterChanged(bool _visible);
    void visualDeckStorageDefaultTagsListChanged();
    void visualDeckStorageShowColorIdentityChanged(bool _visible);
    void visualDeckStorageShowBannerCardComboBoxChanged(bool _visible);
    void visualDeckStorageShowTagsOnDeckPreviewsChanged(bool _visible);
    void visualDeckStorageCardSizeChanged();
    void visualDeckStorageDrawUnusedColorIdentitiesChanged(bool _visible);
    void visualDeckStorageUnusedColorIdentitiesOpacityChanged(bool value);
    void visualDeckStorageInGameChanged(bool enabled);
    void visualDeckStorageSelectionAnimationChanged(bool enabled);
    void visualDatabaseDisplayFilterToMostRecentSetsEnabledChanged(bool enabled);
    void visualDatabaseDisplayFilterToMostRecentSetsAmountChanged(int amount);
    void visualDeckEditorSampleHandSizeAmountChanged(int amount);
    void visualDeckEditorCardSizeChanged();
    void visualDatabaseDisplayCardSizeChanged();
    void edhRecCardSizeChanged();
    void archidektPreviewSizeChanged();
    void horizontalHandChanged();
    void handJustificationChanged();
    void invertVerticalCoordinateChanged();
    void minPlayersForMultiColumnLayoutChanged();
    void soundEnabledChanged();
    void soundThemeChanged();
    void ignoreUnregisteredUsersChanged();
    void ignoreUnregisteredUserMessagesChanged();
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void networkCacheSizeChanged(int newSizeInMBs);
    void redirectCacheTtlChanged(int newTtl);
    void masterVolumeChanged(int value);
    void chatMentionCompleterChanged();
    void downloadSpoilerTimeIndexChanged();
    void downloadSpoilerStatusChanged();
    void useTearOffMenusChanged(bool state);
    void roundCardCornersChanged(bool roundCardCorners);

private:
    QSettings *settings;
    ShortcutsSettings *shortcutsSettings;
    CardDatabaseSettings *cardDatabaseSettings;
    ServersSettings *serversSettings;
    MessageSettings *messageSettings;
    GameFiltersSettings *gameFiltersSettings;
    LayoutsSettings *layoutsSettings;
    DownloadSettings *downloadSettings;
    RecentsSettings *recentsSettings;
    CardOverrideSettings *cardOverrideSettings;
    DebugSettings *debugSettings;
    CardCounterSettings *cardCounterSettings;

    QByteArray mainWindowGeometry;
    QByteArray tokenDialogGeometry;
    QByteArray setsDialogGeometry;
    QString lang;
    QString deckPath, filtersPath, replaysPath, picsPath, redirectCachePath, customPicsPath, cardDatabasePath,
        customCardDatabasePath, themesPath, spoilerDatabasePath, tokenDatabasePath, themeName, homeTabBackgroundSource;
    bool tabVisualDeckStorageOpen, tabServerOpen, tabAccountOpen, tabDeckStorageOpen, tabReplaysOpen, tabAdminOpen,
        tabLogOpen;
    bool checkUpdatesOnStartup;
    bool startupCardUpdateCheckPromptForUpdate;
    bool startupCardUpdateCheckAlwaysUpdate;
    bool checkCardUpdatesOnStartup;
    int cardUpdateCheckInterval;
    QDate lastCardUpdateCheck;
    bool notifyAboutUpdates;
    bool notifyAboutNewVersion;
    bool showTipsOnStartup;
    QList<int> seenTips;
    int homeTabBackgroundShuffleFrequency;
    bool homeTabDisplayCardName;
    bool mbDownloadSpoilers;
    int updateReleaseChannel;
    int maxFontSize;
    bool picDownload;
    bool notificationsEnabled;
    bool spectatorNotificationsEnabled;
    bool buddyConnectNotificationsEnabled;
    bool doubleClickToPlay;
    bool clickPlaysAllSelected;
    bool playToStack;
    bool doNotDeleteArrowsInSubPhases;
    int startingHandSize;
    bool annotateTokens;
    QByteArray tabGameSplitterSizes;
    bool showShortcuts;
    bool showGameSelectorFilterToolbar;
    bool displayCardNames;
    bool overrideAllCardArtWithPersonalPreference;
    bool bumpSetsWithCardsInDeckToTop;
    int printingSelectorSortOrder;
    int printingSelectorCardSize;
    bool includeRebalancedCards;
    bool printingSelectorNavigationButtonsVisible;
    bool deckEditorBannerCardComboBoxVisible;
    bool deckEditorTagsWidgetVisible;
    int visualDeckStorageSortingOrder;
    bool visualDeckStorageShowFolders;
    bool visualDeckStorageShowColorIdentity;
    bool visualDeckStorageShowBannerCardComboBox;
    bool visualDeckStorageShowTagsOnDeckPreviews;
    bool visualDeckStorageShowTagFilter;
    QStringList visualDeckStorageDefaultTagsList;
    bool visualDeckStorageSearchFolderNames;
    int visualDeckStorageCardSize;
    int visualDeckEditorCardSize;
    int visualDatabaseDisplayCardSize;
    int edhrecCardSize;
    int archidektPreviewSize;
    bool visualDeckStorageDrawUnusedColorIdentities;
    int visualDeckStorageUnusedColorIdentitiesOpacity;
    int visualDeckStorageTooltipType;
    bool visualDeckStoragePromptForConversion;
    bool visualDeckStorageAlwaysConvert;
    bool visualDeckStorageInGame;
    bool visualDeckStorageSelectionAnimation;
    int defaultDeckEditorType;
    bool visualDatabaseDisplayFilterToMostRecentSetsEnabled;
    int visualDatabaseDisplayFilterToMostRecentSetsAmount;
    int visualDeckEditorSampleHandSize;
    bool horizontalHand;
    bool invertVerticalCoordinate;
    int minPlayersForMultiColumnLayout;
    bool tapAnimation;
    bool autoRotateSidewaysLayoutCards;
    bool openDeckInNewTab;
    int rewindBufferingMs;
    bool chatMention;
    bool chatMentionCompleter;
    QString chatMentionColor;
    QString chatHighlightColor;
    bool chatMentionForeground;
    bool chatHighlightForeground;
    int zoneViewSortByIndex, zoneViewGroupByIndex;
    bool zoneViewPileView;
    bool soundEnabled;
    QString soundThemeName;
    bool ignoreUnregisteredUsers;
    bool ignoreUnregisteredUserMessages;
    QString picUrl;
    QString picUrlFallback;
    QString clientID;
    QString clientVersion;
    QString knownMissingFeatures;
    bool useTearOffMenus;
    int cardViewInitialRowsMax;
    int cardViewExpandedRowsMax;
    bool closeEmptyCardView;
    bool focusCardViewSearchBar;
    int pixmapCacheSize;
    int networkCacheSize;
    int redirectCacheTtl;
    bool scaleCards;
    int verticalCardOverlapPercent;
    bool showMessagePopups;
    bool showMentionPopups;
    bool roomHistory;
    bool leftJustified;
    int masterVolume;
    int cardInfoViewMode;
    QString highlightWords;
    QString gameDescription;
    int maxPlayers;
    QString gameTypes;
    bool onlyBuddies;
    bool onlyRegistered;
    bool spectatorsAllowed;
    bool spectatorsNeedPassword;
    bool spectatorsCanTalk;
    bool spectatorsCanSeeEverything;
    bool createGameAsSpectator;
    int defaultStartingLifeTotal;
    bool shareDecklistsOnLoad;
    int keepalive;
    int timeout;
    void translateLegacySettings();
    [[nodiscard]] QString getSafeConfigPath(QString configEntry, QString defaultPath) const;
    [[nodiscard]] QString getSafeConfigFilePath(QString configEntry, QString defaultPath) const;
    void loadPaths();
    bool rememberGameSettings;
    QList<ReleaseChannel *> releaseChannels;
    bool isPortableBuild;
    bool roundCardCorners;
    bool showStatusBar;

public:
    SettingsCache();
    QString getDataPath();
    QString getSettingsPath();
    [[nodiscard]] QString getCachePath() const;
    [[nodiscard]] QString getNetworkCachePath() const;
    [[nodiscard]] const QByteArray &getMainWindowGeometry() const
    {
        return mainWindowGeometry;
    }
    [[nodiscard]] const QByteArray &getTokenDialogGeometry() const
    {
        return tokenDialogGeometry;
    }
    [[nodiscard]] const QByteArray &getSetsDialogGeometry() const
    {
        return setsDialogGeometry;
    }
    [[nodiscard]] QString getLang() const
    {
        return lang;
    }
    [[nodiscard]] QString getDeckPath() const
    {
        return deckPath;
    }
    [[nodiscard]] QString getFiltersPath() const
    {
        return filtersPath;
    }
    [[nodiscard]] QString getReplaysPath() const
    {
        return replaysPath;
    }
    [[nodiscard]] QString getThemesPath() const
    {
        return themesPath;
    }
    [[nodiscard]] QString getPicsPath() const
    {
        return picsPath;
    }
    [[nodiscard]] QString getRedirectCachePath() const
    {
        return redirectCachePath;
    }
    [[nodiscard]] QString getCustomPicsPath() const
    {
        return customPicsPath;
    }
    [[nodiscard]] QString getCustomCardDatabasePath() const override
    {
        return customCardDatabasePath;
    }
    [[nodiscard]] QString getCardDatabasePath() const override
    {
        return cardDatabasePath;
    }
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override
    {
        return spoilerDatabasePath;
    }
    [[nodiscard]] QString getTokenDatabasePath() const override
    {
        return tokenDatabasePath;
    }
    [[nodiscard]] QString getThemeName() const
    {
        return themeName;
    }
    [[nodiscard]] QString getHomeTabBackgroundSource() const
    {
        return homeTabBackgroundSource;
    }
    [[nodiscard]] int getHomeTabBackgroundShuffleFrequency() const
    {
        return homeTabBackgroundShuffleFrequency;
    }
    [[nodiscard]] bool getHomeTabDisplayCardName() const
    {
        return homeTabDisplayCardName;
    }
    [[nodiscard]] bool getTabVisualDeckStorageOpen() const
    {
        return tabVisualDeckStorageOpen;
    }
    [[nodiscard]] bool getTabServerOpen() const
    {
        return tabServerOpen;
    }
    [[nodiscard]] bool getTabAccountOpen() const
    {
        return tabAccountOpen;
    }
    [[nodiscard]] bool getTabDeckStorageOpen() const
    {
        return tabDeckStorageOpen;
    }
    [[nodiscard]] bool getTabReplaysOpen() const
    {
        return tabReplaysOpen;
    }
    [[nodiscard]] bool getTabAdminOpen() const
    {
        return tabAdminOpen;
    }
    [[nodiscard]] bool getTabLogOpen() const
    {
        return tabLogOpen;
    }
    [[nodiscard]] QString getChatMentionColor() const
    {
        return chatMentionColor;
    }
    [[nodiscard]] QString getChatHighlightColor() const
    {
        return chatHighlightColor;
    }
    [[nodiscard]] bool getPicDownload() const
    {
        return picDownload;
    }
    [[nodiscard]] bool getShowStatusBar() const
    {
        return showStatusBar;
    }
    [[nodiscard]] bool getNotificationsEnabled() const
    {
        return notificationsEnabled;
    }
    [[nodiscard]] bool getSpectatorNotificationsEnabled() const
    {
        return spectatorNotificationsEnabled;
    }
    [[nodiscard]] bool getBuddyConnectNotificationsEnabled() const
    {
        return buddyConnectNotificationsEnabled;
    }
    [[nodiscard]] bool getCheckUpdatesOnStartup() const
    {
        return checkUpdatesOnStartup;
    }
    bool getStartupCardUpdateCheckPromptForUpdate()
    {
        return startupCardUpdateCheckPromptForUpdate;
    }
    bool getStartupCardUpdateCheckAlwaysUpdate()
    {
        return startupCardUpdateCheckAlwaysUpdate;
    }
    [[nodiscard]] int getCardUpdateCheckInterval() const
    {
        return cardUpdateCheckInterval;
    }
    [[nodiscard]] QDate getLastCardUpdateCheck() const
    {
        return lastCardUpdateCheck;
    }
    [[nodiscard]] bool getCardUpdateCheckRequired() const
    {
        return getLastCardUpdateCheck().daysTo(QDateTime::currentDateTime().date()) >= getCardUpdateCheckInterval() &&
               getLastCardUpdateCheck() != QDateTime::currentDateTime().date();
    }
    [[nodiscard]] bool getNotifyAboutUpdates() const override
    {
        return notifyAboutUpdates;
    }
    [[nodiscard]] bool getNotifyAboutNewVersion() const
    {
        return notifyAboutNewVersion;
    }
    [[nodiscard]] bool getShowTipsOnStartup() const
    {
        return showTipsOnStartup;
    }
    [[nodiscard]] QList<int> getSeenTips() const
    {
        return seenTips;
    }
    [[nodiscard]] int getUpdateReleaseChannelIndex() const
    {
        return updateReleaseChannel;
    }
    [[nodiscard]] ReleaseChannel *getUpdateReleaseChannel() const
    {
        return releaseChannels.at(qMax(0, updateReleaseChannel));
    }
    [[nodiscard]] QList<ReleaseChannel *> getUpdateReleaseChannels() const
    {
        return releaseChannels;
    }

    [[nodiscard]] bool getDoubleClickToPlay() const
    {
        return doubleClickToPlay;
    }
    [[nodiscard]] bool getClickPlaysAllSelected() const
    {
        return clickPlaysAllSelected;
    }
    [[nodiscard]] bool getPlayToStack() const
    {
        return playToStack;
    }
    [[nodiscard]] bool getDoNotDeleteArrowsInSubPhases() const
    {
        return doNotDeleteArrowsInSubPhases;
    }
    [[nodiscard]] int getStartingHandSize() const
    {
        return startingHandSize;
    }
    [[nodiscard]] bool getAnnotateTokens() const
    {
        return annotateTokens;
    }
    [[nodiscard]] QByteArray getTabGameSplitterSizes() const
    {
        return tabGameSplitterSizes;
    }
    [[nodiscard]] bool getShowShortcuts() const
    {
        return showShortcuts;
    }
    [[nodiscard]] bool getShowGameSelectorFilterToolbar() const
    {
        return showGameSelectorFilterToolbar;
    }
    [[nodiscard]] bool getDisplayCardNames() const
    {
        return displayCardNames;
    }
    [[nodiscard]] bool getOverrideAllCardArtWithPersonalPreference() const
    {
        return overrideAllCardArtWithPersonalPreference;
    }
    [[nodiscard]] bool getBumpSetsWithCardsInDeckToTop() const
    {
        return bumpSetsWithCardsInDeckToTop;
    }
    [[nodiscard]] int getPrintingSelectorSortOrder() const
    {
        return printingSelectorSortOrder;
    }
    [[nodiscard]] int getPrintingSelectorCardSize() const
    {
        return printingSelectorCardSize;
    }
    [[nodiscard]] bool getIncludeRebalancedCards() const
    {
        return includeRebalancedCards;
    }
    [[nodiscard]] bool getPrintingSelectorNavigationButtonsVisible() const
    {
        return printingSelectorNavigationButtonsVisible;
    }
    [[nodiscard]] bool getDeckEditorBannerCardComboBoxVisible() const
    {
        return deckEditorBannerCardComboBoxVisible;
    }
    [[nodiscard]] bool getDeckEditorTagsWidgetVisible() const
    {
        return deckEditorTagsWidgetVisible;
    }
    [[nodiscard]] int getVisualDeckStorageSortingOrder() const
    {
        return visualDeckStorageSortingOrder;
    }
    [[nodiscard]] bool getVisualDeckStorageShowFolders() const
    {
        return visualDeckStorageShowFolders;
    }
    [[nodiscard]] bool getVisualDeckStorageShowTagFilter() const
    {
        return visualDeckStorageShowTagFilter;
    }
    [[nodiscard]] QStringList getVisualDeckStorageDefaultTagsList() const
    {
        return visualDeckStorageDefaultTagsList;
    }
    [[nodiscard]] bool getVisualDeckStorageSearchFolderNames() const
    {
        return visualDeckStorageSearchFolderNames;
    }
    [[nodiscard]] bool getVisualDeckStorageShowColorIdentity() const
    {
        return visualDeckStorageShowColorIdentity;
    }
    [[nodiscard]] bool getVisualDeckStorageShowBannerCardComboBox() const
    {
        return visualDeckStorageShowBannerCardComboBox;
    }
    [[nodiscard]] bool getVisualDeckStorageShowTagsOnDeckPreviews() const
    {
        return visualDeckStorageShowTagsOnDeckPreviews;
    }
    [[nodiscard]] int getVisualDeckStorageCardSize() const
    {
        return visualDeckStorageCardSize;
    }
    [[nodiscard]] bool getVisualDeckStorageDrawUnusedColorIdentities() const
    {
        return visualDeckStorageDrawUnusedColorIdentities;
    }
    [[nodiscard]] int getVisualDeckStorageUnusedColorIdentitiesOpacity() const
    {
        return visualDeckStorageUnusedColorIdentitiesOpacity;
    }
    [[nodiscard]] int getVisualDeckStorageTooltipType() const
    {
        return visualDeckStorageTooltipType;
    }
    [[nodiscard]] bool getVisualDeckStoragePromptForConversion() const
    {
        return visualDeckStoragePromptForConversion;
    }
    [[nodiscard]] bool getVisualDeckStorageAlwaysConvert() const
    {
        return visualDeckStorageAlwaysConvert;
    }
    [[nodiscard]] bool getVisualDeckStorageInGame() const
    {
        return visualDeckStorageInGame;
    }
    [[nodiscard]] bool getVisualDeckStorageSelectionAnimation() const
    {
        return visualDeckStorageSelectionAnimation;
    }
    [[nodiscard]] int getVisualDeckEditorCardSize() const
    {
        return visualDeckEditorCardSize;
    }
    [[nodiscard]] int getVisualDatabaseDisplayCardSize() const
    {
        return visualDatabaseDisplayCardSize;
    }
    [[nodiscard]] int getEDHRecCardSize() const
    {
        return edhrecCardSize;
    }
    [[nodiscard]] int getArchidektPreviewSize() const
    {
        return archidektPreviewSize;
    }
    [[nodiscard]] int getDefaultDeckEditorType() const
    {
        return defaultDeckEditorType;
    }
    [[nodiscard]] bool getVisualDatabaseDisplayFilterToMostRecentSetsEnabled() const
    {
        return visualDatabaseDisplayFilterToMostRecentSetsEnabled;
    }
    [[nodiscard]] int getVisualDatabaseDisplayFilterToMostRecentSetsAmount() const
    {
        return visualDatabaseDisplayFilterToMostRecentSetsAmount;
    }
    [[nodiscard]] int getVisualDeckEditorSampleHandSize() const
    {
        return visualDeckEditorSampleHandSize;
    }
    [[nodiscard]] bool getHorizontalHand() const
    {
        return horizontalHand;
    }
    [[nodiscard]] bool getInvertVerticalCoordinate() const
    {
        return invertVerticalCoordinate;
    }
    [[nodiscard]] int getMinPlayersForMultiColumnLayout() const
    {
        return minPlayersForMultiColumnLayout;
    }
    [[nodiscard]] bool getTapAnimation() const
    {
        return tapAnimation;
    }
    [[nodiscard]] bool getAutoRotateSidewaysLayoutCards() const
    {
        return autoRotateSidewaysLayoutCards;
    }
    [[nodiscard]] bool getOpenDeckInNewTab() const
    {
        return openDeckInNewTab;
    }
    [[nodiscard]] int getRewindBufferingMs() const
    {
        return rewindBufferingMs;
    }
    [[nodiscard]] bool getChatMention() const
    {
        return chatMention;
    }
    [[nodiscard]] bool getChatMentionCompleter() const
    {
        return chatMentionCompleter;
    }
    [[nodiscard]] bool getChatMentionForeground() const
    {
        return chatMentionForeground;
    }
    [[nodiscard]] bool getChatHighlightForeground() const
    {
        return chatHighlightForeground;
    }
    /**
     * Currently selected index for the `Group by X` QComboBox
     */
    [[nodiscard]] int getZoneViewGroupByIndex() const
    {
        return zoneViewGroupByIndex;
    }
    /**
     * Currently selected index for the `Sort by X` QComboBox
     */
    [[nodiscard]] int getZoneViewSortByIndex() const
    {
        return zoneViewSortByIndex;
    }
    /**
       Returns if the view should be sorted into pile view.
       @return zoneViewPileView if the view should be sorted into pile view.
     */
    [[nodiscard]] bool getZoneViewPileView() const
    {
        return zoneViewPileView;
    }
    [[nodiscard]] bool getSoundEnabled() const
    {
        return soundEnabled;
    }
    [[nodiscard]] QString getSoundThemeName() const
    {
        return soundThemeName;
    }
    [[nodiscard]] bool getIgnoreUnregisteredUsers() const
    {
        return ignoreUnregisteredUsers;
    }
    [[nodiscard]] bool getIgnoreUnregisteredUserMessages() const
    {
        return ignoreUnregisteredUserMessages;
    }
    [[nodiscard]] int getPixmapCacheSize() const
    {
        return pixmapCacheSize;
    }
    [[nodiscard]] int getNetworkCacheSizeInMB() const
    {
        return networkCacheSize;
    }
    [[nodiscard]] int getRedirectCacheTtl() const
    {
        return redirectCacheTtl;
    }
    [[nodiscard]] bool getScaleCards() const
    {
        return scaleCards;
    }
    [[nodiscard]] int getStackCardOverlapPercent() const
    {
        return verticalCardOverlapPercent;
    }
    [[nodiscard]] bool getShowMessagePopup() const
    {
        return showMessagePopups;
    }
    [[nodiscard]] bool getShowMentionPopup() const
    {
        return showMentionPopups;
    }
    [[nodiscard]] bool getRoomHistory() const
    {
        return roomHistory;
    }
    [[nodiscard]] bool getLeftJustified() const
    {
        return leftJustified;
    }
    [[nodiscard]] int getMasterVolume() const
    {
        return masterVolume;
    }
    [[nodiscard]] int getCardInfoViewMode() const
    {
        return cardInfoViewMode;
    }
    [[nodiscard]] QStringList getCountries() const;
    [[nodiscard]] QString getHighlightWords() const
    {
        return highlightWords;
    }
    [[nodiscard]] QString getGameDescription() const
    {
        return gameDescription;
    }
    [[nodiscard]] int getMaxPlayers() const
    {
        return maxPlayers;
    }
    [[nodiscard]] QString getGameTypes() const
    {
        return gameTypes;
    }
    [[nodiscard]] bool getOnlyBuddies() const
    {
        return onlyBuddies;
    }
    [[nodiscard]] bool getOnlyRegistered() const
    {
        return onlyRegistered;
    }
    [[nodiscard]] bool getSpectatorsAllowed() const
    {
        return spectatorsAllowed;
    }
    [[nodiscard]] bool getSpectatorsNeedPassword() const
    {
        return spectatorsNeedPassword;
    }
    [[nodiscard]] bool getSpectatorsCanTalk() const
    {
        return spectatorsCanTalk;
    }
    [[nodiscard]] bool getSpectatorsCanSeeEverything() const
    {
        return spectatorsCanSeeEverything;
    }
    [[nodiscard]] int getDefaultStartingLifeTotal() const
    {
        return defaultStartingLifeTotal;
    }
    [[nodiscard]] bool getShareDecklistsOnLoad() const
    {
        return shareDecklistsOnLoad;
    }
    [[nodiscard]] bool getCreateGameAsSpectator() const
    {
        return createGameAsSpectator;
    }
    [[nodiscard]] bool getRememberGameSettings() const
    {
        return rememberGameSettings;
    }
    [[nodiscard]] int getKeepAlive() const override
    {
        return keepalive;
    }
    [[nodiscard]] int getTimeOut() const override
    {
        return timeout;
    }
    [[nodiscard]] int getMaxFontSize() const
    {
        return maxFontSize;
    }
    void setClientID(const QString &clientID);
    void setClientVersion(const QString &clientVersion);
    void setKnownMissingFeatures(const QString &_knownMissingFeatures) override;
    void setUseTearOffMenus(bool _useTearOffMenus);
    void setCardViewInitialRowsMax(int _cardViewInitialRowsMax);
    void setCardViewExpandedRowsMax(int value);
    void setCloseEmptyCardView(QT_STATE_CHANGED_T value);
    void setFocusCardViewSearchBar(QT_STATE_CHANGED_T value);
    QString getClientID() override
    {
        return clientID;
    }
    QString getClientVersion()
    {
        return clientVersion;
    }
    QString getKnownMissingFeatures() override
    {
        return knownMissingFeatures;
    }
    bool getUseTearOffMenus()
    {
        return useTearOffMenus;
    }
    [[nodiscard]] int getCardViewInitialRowsMax() const
    {
        return cardViewInitialRowsMax;
    }
    [[nodiscard]] int getCardViewExpandedRowsMax() const
    {
        return cardViewExpandedRowsMax;
    }
    [[nodiscard]] bool getCloseEmptyCardView() const
    {
        return closeEmptyCardView;
    }
    [[nodiscard]] bool getFocusCardViewSearchBar() const
    {
        return focusCardViewSearchBar;
    }
    [[nodiscard]] ShortcutsSettings &shortcuts() const
    {
        return *shortcutsSettings;
    }
    [[nodiscard]] CardDatabaseSettings *cardDatabase() const
    {
        return cardDatabaseSettings;
    }
    [[nodiscard]] ServersSettings &servers() const
    {
        return *serversSettings;
    }
    [[nodiscard]] MessageSettings &messages() const
    {
        return *messageSettings;
    }
    [[nodiscard]] GameFiltersSettings &gameFilters() const
    {
        return *gameFiltersSettings;
    }
    [[nodiscard]] LayoutsSettings &layouts() const
    {
        return *layoutsSettings;
    }
    [[nodiscard]] DownloadSettings &downloads() const
    {
        return *downloadSettings;
    }
    [[nodiscard]] RecentsSettings &recents() const
    {
        return *recentsSettings;
    }
    [[nodiscard]] CardOverrideSettings &cardOverrides() const
    {
        return *cardOverrideSettings;
    }
    [[nodiscard]] DebugSettings &debug() const
    {
        return *debugSettings;
    }
    [[nodiscard]] CardCounterSettings &cardCounters() const;

    [[nodiscard]] bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }
    [[nodiscard]] bool getDownloadSpoilersStatus() const
    {
        return mbDownloadSpoilers;
    }
    [[nodiscard]] bool getRoundCardCorners() const
    {
        return roundCardCorners;
    }

    static SettingsCache &instance();
    void resetPaths();

public slots:
    void setDownloadSpoilerStatus(bool _spoilerStatus);

    void setMainWindowGeometry(const QByteArray &_mainWindowGeometry);
    void setTokenDialogGeometry(const QByteArray &_tokenDialog);
    void setSetsDialogGeometry(const QByteArray &_setsDialog);
    void setLang(const QString &_lang);
    void setShowTipsOnStartup(bool _showTipsOnStartup);
    void setSeenTips(const QList<int> &_seenTips);
    void setDeckPath(const QString &_deckPath);
    void setFiltersPath(const QString &_filtersPath);
    void setReplaysPath(const QString &_replaysPath);
    void setThemesPath(const QString &_themesPath);
    void setCustomCardDatabasePath(const QString &_customCardDatabasePath);
    void setPicsPath(const QString &_picsPath);
    void setCardDatabasePath(const QString &_cardDatabasePath);
    void setSpoilerDatabasePath(const QString &_spoilerDatabasePath);
    void setTokenDatabasePath(const QString &_tokenDatabasePath);
    void setThemeName(const QString &_themeName);
    void setHomeTabBackgroundSource(const QString &_backgroundSource);
    void setHomeTabBackgroundShuffleFrequency(int _frequency);
    void setHomeTabDisplayCardName(QT_STATE_CHANGED_T _displayCardName);
    void setTabVisualDeckStorageOpen(bool value);
    void setTabServerOpen(bool value);
    void setTabAccountOpen(bool value);
    void setTabDeckStorageOpen(bool value);
    void setTabReplaysOpen(bool value);
    void setTabAdminOpen(bool value);
    void setTabLogOpen(bool value);
    void setChatMentionColor(const QString &_chatMentionColor);
    void setChatHighlightColor(const QString &_chatHighlightColor);
    void setPicDownload(QT_STATE_CHANGED_T _picDownload);
    void setShowStatusBar(bool value);
    void setNotificationsEnabled(QT_STATE_CHANGED_T _notificationsEnabled);
    void setSpectatorNotificationsEnabled(QT_STATE_CHANGED_T _spectatorNotificationsEnabled);
    void setBuddyConnectNotificationsEnabled(QT_STATE_CHANGED_T _buddyConnectNotificationsEnabled);
    void setDoubleClickToPlay(QT_STATE_CHANGED_T _doubleClickToPlay);
    void setClickPlaysAllSelected(QT_STATE_CHANGED_T _clickPlaysAllSelected);
    void setPlayToStack(QT_STATE_CHANGED_T _playToStack);
    void setDoNotDeleteArrowsInSubPhases(QT_STATE_CHANGED_T _doNotDeleteArrowsInSubPhases);
    void setStartingHandSize(int _startingHandSize);
    void setAnnotateTokens(QT_STATE_CHANGED_T _annotateTokens);
    void setTabGameSplitterSizes(const QByteArray &_tabGameSplitterSizes);
    void setShowShortcuts(QT_STATE_CHANGED_T _showShortcuts);
    void setShowGameSelectorFilterToolbar(QT_STATE_CHANGED_T _showGameSelectorFilterToolbar);
    void setDisplayCardNames(QT_STATE_CHANGED_T _displayCardNames);
    void setOverrideAllCardArtWithPersonalPreference(QT_STATE_CHANGED_T _overrideAllCardArt);
    void setBumpSetsWithCardsInDeckToTop(QT_STATE_CHANGED_T _bumpSetsWithCardsInDeckToTop);
    void setPrintingSelectorSortOrder(int _printingSelectorSortOrder);
    void setPrintingSelectorCardSize(int _printingSelectorCardSize);
    void setIncludeRebalancedCards(bool _includeRebalancedCards);
    void setPrintingSelectorNavigationButtonsVisible(QT_STATE_CHANGED_T _navigationButtonsVisible);
    void setDeckEditorBannerCardComboBoxVisible(QT_STATE_CHANGED_T _deckEditorBannerCardComboBoxVisible);
    void setDeckEditorTagsWidgetVisible(QT_STATE_CHANGED_T _deckEditorTagsWidgetVisible);
    void setVisualDeckStorageSortingOrder(int _visualDeckStorageSortingOrder);
    void setVisualDeckStorageShowFolders(QT_STATE_CHANGED_T value);
    void setVisualDeckStorageShowTagFilter(QT_STATE_CHANGED_T _showTags);
    void setVisualDeckStorageDefaultTagsList(QStringList _defaultTagsList);
    void setVisualDeckStorageSearchFolderNames(QT_STATE_CHANGED_T value);
    void setVisualDeckStorageShowColorIdentity(QT_STATE_CHANGED_T value);
    void setVisualDeckStorageShowBannerCardComboBox(QT_STATE_CHANGED_T _showBannerCardComboBox);
    void setVisualDeckStorageShowTagsOnDeckPreviews(QT_STATE_CHANGED_T _showTags);
    void setVisualDeckStorageCardSize(int _visualDeckStorageCardSize);
    void setVisualDeckStorageDrawUnusedColorIdentities(QT_STATE_CHANGED_T _visualDeckStorageDrawUnusedColorIdentities);
    void setVisualDeckStorageUnusedColorIdentitiesOpacity(int _visualDeckStorageUnusedColorIdentitiesOpacity);
    void setVisualDeckStorageTooltipType(int value);
    void setVisualDeckStoragePromptForConversion(bool _visualDeckStoragePromptForConversion);
    void setVisualDeckStorageAlwaysConvert(bool _visualDeckStorageAlwaysConvert);
    void setVisualDeckStorageInGame(QT_STATE_CHANGED_T value);
    void setVisualDeckStorageSelectionAnimation(QT_STATE_CHANGED_T value);
    void setVisualDeckEditorCardSize(int _visualDeckEditorCardSize);
    void setVisualDatabaseDisplayCardSize(int _visualDatabaseDisplayCardSize);
    void setEDHRecCardSize(int _EDHRecCardSize);
    void setArchidektPreviewCardSize(int _archidektPreviewCardSize);
    void setDefaultDeckEditorType(int value);
    void setVisualDatabaseDisplayFilterToMostRecentSetsEnabled(QT_STATE_CHANGED_T _enabled);
    void setVisualDatabaseDisplayFilterToMostRecentSetsAmount(int _amount);
    void setVisualDeckEditorSampleHandSize(int _amount);
    void setHorizontalHand(QT_STATE_CHANGED_T _horizontalHand);
    void setInvertVerticalCoordinate(QT_STATE_CHANGED_T _invertVerticalCoordinate);
    void setMinPlayersForMultiColumnLayout(int _minPlayersForMultiColumnLayout);
    void setTapAnimation(QT_STATE_CHANGED_T _tapAnimation);
    void setAutoRotateSidewaysLayoutCards(QT_STATE_CHANGED_T _autoRotateSidewaysLayoutCards);
    void setOpenDeckInNewTab(QT_STATE_CHANGED_T _openDeckInNewTab);
    void setRewindBufferingMs(int _rewindBufferingMs);
    void setChatMention(QT_STATE_CHANGED_T _chatMention);
    void setChatMentionCompleter(QT_STATE_CHANGED_T _chatMentionCompleter);
    void setChatMentionForeground(QT_STATE_CHANGED_T _chatMentionForeground);
    void setChatHighlightForeground(QT_STATE_CHANGED_T _chatHighlightForeground);
    void setZoneViewGroupByIndex(const int _zoneViewGroupByIndex);
    void setZoneViewSortByIndex(const int _zoneViewSortByIndex);
    void setZoneViewPileView(QT_STATE_CHANGED_T _zoneViewPileView);
    void setSoundEnabled(QT_STATE_CHANGED_T _soundEnabled);
    void setSoundThemeName(const QString &_soundThemeName);
    void setIgnoreUnregisteredUsers(QT_STATE_CHANGED_T _ignoreUnregisteredUsers);
    void setIgnoreUnregisteredUserMessages(QT_STATE_CHANGED_T _ignoreUnregisteredUserMessages);
    void setPixmapCacheSize(const int _pixmapCacheSize);
    void setNetworkCacheSizeInMB(const int _networkCacheSize);
    void setNetworkRedirectCacheTtl(const int _redirectCacheTtl);
    void setCardScaling(const QT_STATE_CHANGED_T _scaleCards);
    void setStackCardOverlapPercent(const int _verticalCardOverlapPercent);
    void setShowMessagePopups(const QT_STATE_CHANGED_T _showMessagePopups);
    void setShowMentionPopups(const QT_STATE_CHANGED_T _showMentionPopups);
    void setRoomHistory(const QT_STATE_CHANGED_T _roomHistory);
    void setLeftJustified(const QT_STATE_CHANGED_T _leftJustified);
    void setMasterVolume(const int _masterVolume);
    void setCardInfoViewMode(const int _viewMode);
    void setHighlightWords(const QString &_highlightWords);
    void setGameDescription(const QString _gameDescription);
    void setMaxPlayers(const int _maxPlayers);
    void setGameTypes(const QString _gameTypes);
    void setOnlyBuddies(const bool _onlyBuddies);
    void setOnlyRegistered(const bool _onlyRegistered);
    void setSpectatorsAllowed(const bool _spectatorsAllowed);
    void setSpectatorsNeedPassword(const bool _spectatorsNeedPassword);
    void setSpectatorsCanTalk(const bool _spectatorsCanTalk);
    void setSpectatorsCanSeeEverything(const bool _spectatorsCanSeeEverything);
    void setCreateGameAsSpectator(const bool _createGameAsSpectator);
    void setDefaultStartingLifeTotal(const int _defaultStartingLifeTotal);
    void setShareDecklistsOnLoad(const bool _shareDecklistsOnLoad);
    void setRememberGameSettings(const bool _rememberGameSettings);
    void setCheckUpdatesOnStartup(QT_STATE_CHANGED_T value);
    void setStartupCardUpdateCheckPromptForUpdate(bool value);
    void setStartupCardUpdateCheckAlwaysUpdate(bool value);
    void setCardUpdateCheckInterval(int value);
    void setLastCardUpdateCheck(QDate value);
    void setNotifyAboutUpdate(QT_STATE_CHANGED_T _notifyaboutupdate);
    void setNotifyAboutNewVersion(QT_STATE_CHANGED_T _notifyaboutnewversion);
    void setUpdateReleaseChannelIndex(int value);
    void setMaxFontSize(int _max);
    void setRoundCardCorners(bool _roundCardCorners);
};
#endif
