#include "gtest/gtest.h"
#include <QSettings>
#include <QTemporaryDir>
#include <libcockatrice/settings/appearance_settings.h>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/chat_settings.h>
#include <libcockatrice/settings/deck_editor_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/game_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/network_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/sound_settings.h>
#include <libcockatrice/settings/tabs_settings.h>
#include <libcockatrice/settings/updates_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

namespace
{

class SettingsDefaultsTest : public ::testing::Test
{
protected:
    QTemporaryDir tempDir;
    QString settingsPath;

    void SetUp() override
    {
        settingsPath = tempDir.path() + "/";
    }
};

// --- CacheStorageSettings ---

TEST_F(SettingsDefaultsTest, CacheStorage_PixmapCacheSize_Default)
{
    CacheStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getPixmapCacheSize(), PIXMAPCACHE_SIZE_DEFAULT);
}

TEST_F(SettingsDefaultsTest, CacheStorage_PixmapCacheSize_SetAndGet)
{
    CacheStorageSettings s(settingsPath, nullptr);
    s.setPixmapCacheSize(1024);
    ASSERT_EQ(s.getPixmapCacheSize(), 1024);
}

TEST_F(SettingsDefaultsTest, CacheStorage_NetworkCacheSizeInMB_Default)
{
    CacheStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getNetworkCacheSizeInMB(), NETWORK_CACHE_SIZE_DEFAULT);
}

TEST_F(SettingsDefaultsTest, CacheStorage_RedirectCacheTtl_Default)
{
    CacheStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getRedirectCacheTtl(), NETWORK_REDIRECT_CACHE_TTL_DEFAULT);
}

TEST_F(SettingsDefaultsTest, CacheStorage_CardPictureLoaderCacheMethod_Default)
{
    CacheStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getCardPictureLoaderCacheMethod(), 0);
}

TEST_F(SettingsDefaultsTest, CacheStorage_LocalCardImageStorageNamingScheme_Default)
{
    CacheStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getLocalCardImageStorageNamingScheme(), LOCAL_CARD_IMAGE_NAMING_SCHEME_DEFAULT);
}

// --- GameSettings ---

TEST_F(SettingsDefaultsTest, Game_MaxPlayers_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getMaxPlayers(), 2);
}

TEST_F(SettingsDefaultsTest, Game_MaxPlayers_SetAndGet)
{
    GameSettings s(settingsPath, nullptr);
    s.setMaxPlayers(4);
    ASSERT_EQ(s.getMaxPlayers(), 4);
}

TEST_F(SettingsDefaultsTest, Game_DefaultStartingLifeTotal_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getDefaultStartingLifeTotal(), 20);
}

TEST_F(SettingsDefaultsTest, Game_RememberGameSettings_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getRememberGameSettings(), true);
}

TEST_F(SettingsDefaultsTest, Game_LocalGameMaxPlayers_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getLocalGameMaxPlayers(), 1);
}

TEST_F(SettingsDefaultsTest, Game_LocalGameStartingLifeTotal_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getLocalGameStartingLifeTotal(), 20);
}

TEST_F(SettingsDefaultsTest, Game_LocalGameRememberSettings_Default)
{
    GameSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getLocalGameRememberSettings(), false);
}

// --- UpdatesSettings ---

TEST_F(SettingsDefaultsTest, Updates_CheckUpdatesOnStartup_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getCheckUpdatesOnStartup(), true);
}

TEST_F(SettingsDefaultsTest, Updates_StartupCardUpdateCheckPromptForUpdate_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getStartupCardUpdateCheckPromptForUpdate(), true);
}

TEST_F(SettingsDefaultsTest, Updates_StartupCardUpdateCheckAlwaysUpdate_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getStartupCardUpdateCheckAlwaysUpdate(), false);
}

TEST_F(SettingsDefaultsTest, Updates_CardUpdateCheckInterval_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getCardUpdateCheckInterval(), 7);
}

TEST_F(SettingsDefaultsTest, Updates_AlwaysEnableNewSets_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getAlwaysEnableNewSets(), false);
}

TEST_F(SettingsDefaultsTest, Updates_NotifyAboutUpdates_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getNotifyAboutUpdates(), true);
}

TEST_F(SettingsDefaultsTest, Updates_NotifyAboutNewVersion_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getNotifyAboutNewVersion(), true);
}

TEST_F(SettingsDefaultsTest, Updates_UpdateReleaseChannelIndex_Default)
{
    UpdatesSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getUpdateReleaseChannelIndex(), 0);
}

// --- SoundSettings ---

TEST_F(SettingsDefaultsTest, Sound_SoundEnabled_Default)
{
    SoundSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getSoundEnabled(), false);
}

TEST_F(SettingsDefaultsTest, Sound_MasterVolume_Default)
{
    SoundSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getMasterVolume(), 100);
}

TEST_F(SettingsDefaultsTest, Sound_MasterVolume_SetAndGet)
{
    SoundSettings s(settingsPath, nullptr);
    s.setMasterVolume(50);
    ASSERT_EQ(s.getMasterVolume(), 50);
}

// --- TabsSettings ---

TEST_F(SettingsDefaultsTest, Tabs_AllTabsOpen_Default)
{
    TabsSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getTabVisualDeckStorageOpen(), true);
    ASSERT_EQ(s.getTabServerOpen(), true);
    ASSERT_EQ(s.getTabAccountOpen(), true);
    ASSERT_EQ(s.getTabDeckStorageOpen(), true);
    ASSERT_EQ(s.getTabReplaysOpen(), true);
    ASSERT_EQ(s.getTabAdminOpen(), true);
    ASSERT_EQ(s.getTabLogOpen(), true);
}

// --- ChatSettings ---

TEST_F(SettingsDefaultsTest, Chat_Mention_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatMention(), true);
}

TEST_F(SettingsDefaultsTest, Chat_MentionCompleter_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatMentionCompleter(), true);
}

TEST_F(SettingsDefaultsTest, Chat_MentionColor_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatMentionColor(), QString("A6120D"));
}

TEST_F(SettingsDefaultsTest, Chat_HighlightColor_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatHighlightColor(), QString("A6120D"));
}

TEST_F(SettingsDefaultsTest, Chat_MentionForeground_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatMentionForeground(), true);
}

TEST_F(SettingsDefaultsTest, Chat_HighlightForeground_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getChatHighlightForeground(), true);
}

TEST_F(SettingsDefaultsTest, Chat_ShowMessagePopup_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowMessagePopup(), true);
}

TEST_F(SettingsDefaultsTest, Chat_ShowMentionPopup_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowMentionPopup(), true);
}

TEST_F(SettingsDefaultsTest, Chat_RoomHistory_Default)
{
    ChatSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getRoomHistory(), true);
}

// --- PersonalSettings ---

TEST_F(SettingsDefaultsTest, Personal_Lang_Default)
{
    PersonalSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getLang(), QString(""));
}

TEST_F(SettingsDefaultsTest, Personal_ShowTipsOnStartup_Default)
{
    PersonalSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowTipsOnStartup(), true);
}

// --- DownloadSettings ---

TEST_F(SettingsDefaultsTest, Download_PicDownload_Default)
{
    DownloadSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getPicDownload(), true);
}

TEST_F(SettingsDefaultsTest, Download_DownloadSpoilersStatus_Default)
{
    DownloadSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getDownloadSpoilersStatus(), false);
}

// --- AppearanceSettings ---

TEST_F(SettingsDefaultsTest, Appearance_ThemeName_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getThemeName(), QString(""));
}

TEST_F(SettingsDefaultsTest, Appearance_ThemeName_SetAndGet)
{
    AppearanceSettings s(settingsPath, nullptr);
    s.setThemeName("my_theme");
    ASSERT_EQ(s.getThemeName(), QString("my_theme"));
}

TEST_F(SettingsDefaultsTest, Appearance_StyleUserList_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getStyleUserList(), true);
}

TEST_F(SettingsDefaultsTest, Appearance_MaxFontSize_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getMaxFontSize(), 12);
}

TEST_F(SettingsDefaultsTest, Appearance_MaxFontSize_SetAndGet)
{
    AppearanceSettings s(settingsPath, nullptr);
    s.setMaxFontSize(14);
    ASSERT_EQ(s.getMaxFontSize(), 14);
}

TEST_F(SettingsDefaultsTest, Appearance_HomeTabBackgroundSource_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getHomeTabBackgroundSource(), QString("themed"));
}

TEST_F(SettingsDefaultsTest, Appearance_HomeTabBackgroundShuffleFrequency_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getHomeTabBackgroundShuffleFrequency(), 0);
}

TEST_F(SettingsDefaultsTest, Appearance_HomeTabDisplayCardName_Default)
{
    AppearanceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getHomeTabDisplayCardName(), true);
}

// --- InterfaceSettings ---

TEST_F(SettingsDefaultsTest, Interface_ShowStatusBar_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowStatusBar(), false);
}

TEST_F(SettingsDefaultsTest, Interface_ShowShortcuts_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowShortcuts(), true);
}

TEST_F(SettingsDefaultsTest, Interface_ShowGameSelectorFilterToolbar_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getShowGameSelectorFilterToolbar(), true);
}

TEST_F(SettingsDefaultsTest, Interface_NotificationsEnabled_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getNotificationsEnabled(), true);
}

TEST_F(SettingsDefaultsTest, Interface_SpectatorNotificationsEnabled_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getSpectatorNotificationsEnabled(), false);
}

TEST_F(SettingsDefaultsTest, Interface_BuddyConnectNotificationsEnabled_Default)
{
    InterfaceSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getBuddyConnectNotificationsEnabled(), true);
}

// --- DeckEditorSettings ---

TEST_F(SettingsDefaultsTest, DeckEditor_OpenDeckInNewTab_Default)
{
    DeckEditorSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getOpenDeckInNewTab(), false);
}

TEST_F(SettingsDefaultsTest, DeckEditor_BannerCardComboBoxVisible_Default)
{
    DeckEditorSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getBannerCardComboBoxVisible(), true);
}

TEST_F(SettingsDefaultsTest, DeckEditor_TagsWidgetVisible_Default)
{
    DeckEditorSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getTagsWidgetVisible(), true);
}

TEST_F(SettingsDefaultsTest, DeckEditor_DefaultDeckEditorType_Default)
{
    DeckEditorSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getDefaultDeckEditorType(), 1);
}

TEST_F(SettingsDefaultsTest, DeckEditor_VdeStartupTab_Default)
{
    DeckEditorSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVdeStartupTab(), VdeStartupTabContext);
}

TEST_F(SettingsDefaultsTest, DeckEditor_VdeStartupTab_SetAndGet)
{
    DeckEditorSettings s(settingsPath, nullptr);
    s.setVdeStartupTab(VdeStartupTabDeckDisplay);
    ASSERT_EQ(s.getVdeStartupTab(), VdeStartupTabDeckDisplay);
    s.setVdeStartupTab(VdeStartupTabDatabaseDisplay);
    ASSERT_EQ(s.getVdeStartupTab(), VdeStartupTabDatabaseDisplay);
    s.setVdeStartupTab(VdeStartupTabContext);
    ASSERT_EQ(s.getVdeStartupTab(), VdeStartupTabContext);
}

// --- NetworkSettings ---

TEST_F(SettingsDefaultsTest, Network_ClientID_Default)
{
    NetworkSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getClientID(), QString("notset"));
}

TEST_F(SettingsDefaultsTest, Network_ClientVersion_Default)
{
    NetworkSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getClientVersion(), QString("notset"));
}

TEST_F(SettingsDefaultsTest, Network_KeepAlive_Default)
{
    NetworkSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getKeepAlive(), 3);
}

TEST_F(SettingsDefaultsTest, Network_TimeOut_Default)
{
    NetworkSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getTimeOut(), 5);
}

TEST_F(SettingsDefaultsTest, Network_KnownMissingFeatures_Default)
{
    NetworkSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getKnownMissingFeatures(), QString(""));
}

// --- CardsDisplaySettings ---

TEST_F(SettingsDefaultsTest, CardsDisplay_PrintingSelectorCardSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getPrintingSelectorCardSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_VisualDeckStorageCardSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVisualDeckStorageCardSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_VisualDatabaseDisplayCardSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVisualDatabaseDisplayCardSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_VisualDeckEditorCardSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVisualDeckEditorCardSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_EDHRecCardSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getEDHRecCardSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_ArchidektPreviewSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getArchidektPreviewSize(), 100);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_SampleHandSize_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getSampleHandSize(), 7);
}

TEST_F(SettingsDefaultsTest, CardsDisplay_ArrowDrawAnimation_Default)
{
    CardsDisplaySettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getArrowDrawAnimation(), true);
}

// --- VisualDeckStorageSettings ---

TEST_F(SettingsDefaultsTest, VisualDeckStorage_SortingOrder_Default)
{
    VisualDeckStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVisualDeckStorageSortingOrder(), 0);
}

TEST_F(SettingsDefaultsTest, VisualDeckStorage_ShowFolders_Default)
{
    VisualDeckStorageSettings s(settingsPath, nullptr);
    ASSERT_EQ(s.getVisualDeckStorageShowFolders(), true);
}

TEST_F(SettingsDefaultsTest, VisualDeckStorage_DefaultTagsList_NotEmpty)
{
    VisualDeckStorageSettings s(settingsPath, nullptr);
    ASSERT_GT(s.getVisualDeckStorageDefaultTagsList().size(), 0);
}

TEST_F(SettingsDefaultsTest, VisualDeckStorage_DefaultTagsList_SetAndGet)
{
    VisualDeckStorageSettings s(settingsPath, nullptr);
    QStringList custom = {"Tag1", "Tag2"};
    s.setVisualDeckStorageDefaultTagsList(custom);
    ASSERT_EQ(s.getVisualDeckStorageDefaultTagsList(), custom);
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
