/**
 * @file dlg_settings.h
 * @ingroup Dialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpinBox>
#include <libcockatrice/utility/macros.h>

inline Q_LOGGING_CATEGORY(DlgSettingsLog, "dlg_settings");

class ShortcutTreeView;
class SearchLineEdit;
class QTreeView;
class QStandardItemModel;
class CardDatabase;
class QCloseEvent;
class QGridLayout;
class QHBoxLayout;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QRadioButton;
class QSlider;
class QStackedWidget;
class QVBoxLayout;
class SequenceEdit;

class AbstractSettingsPage : public QWidget
{
public:
    virtual void retranslateUi() = 0;
};

class GeneralSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    GeneralSettingsPage();
    void retranslateUi() override;

private slots:
    void deckPathButtonClicked();
    void filtersPathButtonClicked();
    void replaysPathButtonClicked();
    void picsPathButtonClicked();
    void cardDatabasePathButtonClicked();
    void customCardDatabaseButtonClicked();
    void tokenDatabasePathButtonClicked();
    void resetAllPathsClicked();
    void languageBoxChanged(int index);

private:
    QStringList findQmFiles();
    QString languageName(const QString &lang);
    QLineEdit *deckPathEdit;
    QLineEdit *filtersPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *customCardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QPushButton *resetAllPathsButton;
    QLabel *allPathsResetLabel;
    QGroupBox *personalGroupBox;
    QGroupBox *pathsGroupBox;
    QComboBox languageBox;
    QCheckBox startupUpdateCheckCheckBox;
    QLabel startupCardUpdateCheckBehaviorLabel;
    QComboBox startupCardUpdateCheckBehaviorSelector;
    QLabel cardUpdateCheckIntervalLabel;
    QSpinBox cardUpdateCheckIntervalSpinBox;
    QLabel lastCardUpdateCheckDateLabel;
    QCheckBox updateNotificationCheckBox;
    QCheckBox newVersionOracleCheckBox;
    QComboBox updateReleaseChannelBox;
    QLabel languageLabel;
    QLabel deckPathLabel;
    QLabel filtersPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel customCardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLabel updateReleaseChannelLabel;
    QLabel advertiseTranslationPageLabel;
    QCheckBox showTipsOnStartup;
};

class AppearanceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void themeBoxChanged(int index);
    void openThemeLocation();
    void showShortcutsChanged(QT_STATE_CHANGED_T enabled);
    void overrideAllCardArtWithPersonalPreferenceToggled(QT_STATE_CHANGED_T enabled);

    void cardViewInitialRowsMaxChanged(int value);
    void cardViewExpandedRowsMaxChanged(int value);

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QPushButton openThemeButton;
    QLabel homeTabBackgroundSourceLabel;
    QComboBox homeTabBackgroundSourceBox;
    QLabel homeTabBackgroundShuffleFrequencyLabel;
    QSpinBox homeTabBackgroundShuffleFrequencySpinBox;
    QLabel minPlayersForMultiColumnLayoutLabel;
    QLabel maxFontSizeForCardsLabel;
    QCheckBox showShortcutsCheckBox;
    QCheckBox displayCardNamesCheckBox;
    QCheckBox autoRotateSidewaysLayoutCardsCheckBox;
    QCheckBox overrideAllCardArtWithPersonalPreferenceCheckBox;
    QCheckBox bumpSetsWithCardsInDeckToTopCheckBox;
    QCheckBox cardScalingCheckBox;
    QCheckBox roundCardCornersCheckBox;
    QLabel verticalCardOverlapPercentLabel;
    QSpinBox verticalCardOverlapPercentBox;
    QLabel cardViewInitialRowsMaxLabel;
    QSpinBox cardViewInitialRowsMaxBox;
    QLabel cardViewExpandedRowsMaxLabel;
    QSpinBox cardViewExpandedRowsMaxBox;
    QCheckBox horizontalHandCheckBox;
    QCheckBox leftJustifiedHandCheckBox;
    QCheckBox invertVerticalCoordinateCheckBox;
    QGroupBox *themeGroupBox;
    QGroupBox *menuGroupBox;
    QGroupBox *cardsGroupBox;
    QGroupBox *handGroupBox;
    QGroupBox *tableGroupBox;
    QGroupBox *cardCountersGroupBox;
    QList<QLabel *> cardCounterNames;
    QSpinBox minPlayersForMultiColumnLayoutEdit;
    QSpinBox maxFontSizeForCardsEdit;

public:
    AppearanceSettingsPage();
    void retranslateUi() override;
};

class UserInterfaceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void setNotificationEnabled(QT_STATE_CHANGED_T);

private:
    QCheckBox notificationsEnabledCheckBox;
    QCheckBox specNotificationsEnabledCheckBox;
    QCheckBox buddyConnectNotificationsEnabledCheckBox;
    QCheckBox doubleClickToPlayCheckBox;
    QCheckBox clickPlaysAllSelectedCheckBox;
    QCheckBox playToStackCheckBox;
    QCheckBox doNotDeleteArrowsInSubPhasesCheckBox;
    QCheckBox closeEmptyCardViewCheckBox;
    QCheckBox focusCardViewSearchBarCheckBox;
    QCheckBox annotateTokensCheckBox;
    QCheckBox useTearOffMenusCheckBox;
    QCheckBox tapAnimationCheckBox;
    QCheckBox openDeckInNewTabCheckBox;
    QLabel visualDeckStoragePromptForConversionLabel;
    QComboBox visualDeckStoragePromptForConversionSelector;
    QCheckBox visualDeckStorageInGameCheckBox;
    QCheckBox visualDeckStorageSelectionAnimationCheckBox;
    QLabel defaultDeckEditorTypeLabel;
    QComboBox defaultDeckEditorTypeSelector;
    QLabel rewindBufferingMsLabel;
    QSpinBox rewindBufferingMsBox;
    QGroupBox *generalGroupBox;
    QGroupBox *notificationsGroupBox;
    QGroupBox *animationGroupBox;
    QGroupBox *deckEditorGroupBox;
    QGroupBox *replayGroupBox;

public:
    UserInterfaceSettingsPage();
    void retranslateUi() override;
};

class DeckEditorSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    DeckEditorSettingsPage();
    void retranslateUi() override;
    QString getLastUpdateTime();

private slots:
    void storeSettings();
    void urlListChanged(const QModelIndex &, int, int, const QModelIndex &, int);
    void setSpoilersEnabled(bool);
    void spoilerPathButtonClicked();
    void updateSpoilers();
    void unlockSettings();
    void actAddURL();
    void actRemoveURL();
    void actEditURL();
    void clearDownloadedPicsButtonClicked();
    void resetDownloadedURLsButtonClicked();

private:
    QPushButton clearDownloadedPicsButton;
    QPushButton resetDownloadURLs;
    QLabel urlLinkLabel;
    QCheckBox picDownloadCheckBox;
    QListWidget *urlList;
    QAction *aAdd, *aEdit, *aRemove;
    QCheckBox mcDownloadSpoilersCheckBox;
    QLabel msDownloadSpoilersLabel;
    QGroupBox *mpGeneralGroupBox;
    QGroupBox *mpSpoilerGroupBox;
    QLineEdit *mpSpoilerSavePathLineEdit;
    QLabel mcSpoilerSaveLabel;
    QLabel lastUpdatedLabel;
    QLabel infoOnSpoilersLabel;
    QPushButton *mpSpoilerPathButton;
    QPushButton *updateNowButton;
    QLabel networkCacheLabel;
    QSpinBox networkCacheEdit;
    QLabel networkRedirectCacheTtlLabel;
    QSpinBox networkRedirectCacheTtlEdit;
    QSpinBox pixmapCacheEdit;
    QLabel pixmapCacheLabel;
};

class MessagesSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    MessagesSettingsPage();
    void retranslateUi() override;

private slots:
    void actAdd();
    void actEdit();
    void actRemove();
    void updateColor(const QString &value);
    void updateHighlightColor(const QString &value);
    void updateTextColor(QT_STATE_CHANGED_T value);
    void updateTextHighlightColor(QT_STATE_CHANGED_T value);

private:
    QListWidget *messageList;
    QAction *aAdd;
    QAction *aEdit;
    QAction *aRemove;
    QCheckBox chatMentionCheckBox;
    QCheckBox chatMentionCompleterCheckbox;
    QCheckBox invertMentionForeground;
    QCheckBox invertHighlightForeground;
    QCheckBox ignoreUnregUsersMainChat;
    QCheckBox ignoreUnregUserMessages;
    QCheckBox messagePopups;
    QCheckBox mentionPopups;
    QCheckBox roomHistory;
    QGroupBox *chatGroupBox;
    QGroupBox *highlightGroupBox;
    QGroupBox *messageGroupBox;
    QLineEdit *mentionColor;
    QLineEdit *highlightColor;
    QLineEdit *customAlertString;
    QLabel hexLabel;
    QLabel hexHighlightLabel;
    QLabel customAlertStringLabel;
    QLabel explainMessagesLabel;

    void storeSettings();
    void updateMentionPreview();
    void updateHighlightPreview();
};

class SoundSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    SoundSettingsPage();
    void retranslateUi() override;

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QGroupBox *soundGroupBox;
    QPushButton soundTestButton;
    QCheckBox soundEnabledCheckBox;
    QLabel masterVolumeLabel;
    QSlider *masterVolumeSlider;
    QSpinBox *masterVolumeSpinBox;

private slots:
    void masterVolumeChanged(int value);
    void themeBoxChanged(int index);
};

class ShortcutSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    ShortcutSettingsPage();
    void retranslateUi() override;

private:
    SearchLineEdit *searchEdit;
    ShortcutTreeView *shortcutsTable;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    QGroupBox *editShortcutGroupBox;
    QGridLayout *editLayout;
    QLabel *currentActionGroupLabel;
    QLabel *currentActionGroupName;
    QLabel *currentActionLabel;
    QLabel *currentActionName;
    QLabel *currentShortcutLabel;
    SequenceEdit *editTextBox;
    QLabel *faqLabel;
    QPushButton *btnResetAll;
    QPushButton *btnClearAll;

private slots:
    void resetShortcuts();
    void clearShortcuts();
    void currentItemChanged(const QString &key);
};

class DlgSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DlgSettings(QWidget *parent = nullptr);
    void setTab(int index);

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void updateLanguage();

private:
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    QListWidgetItem *generalButton, *appearanceButton, *userInterfaceButton, *deckEditorButton, *messagesButton,
        *soundButton, *shortcutsButton;
    void createIcons();
    void retranslateUi();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif
