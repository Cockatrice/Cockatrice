#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include "../utility/macros.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

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
class QTreeWidget;
class QTreeWidgetItem;
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
    QCheckBox updateNotificationCheckBox;
    QCheckBox newVersionOracleCheckBox;
    QComboBox updateReleaseChannelBox;
    QLabel languageLabel;
    QLabel deckPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel customCardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLabel updateReleaseChannelLabel;
    QCheckBox showTipsOnStartup;
};

class AppearanceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void themeBoxChanged(int index);
    void openThemeLocation();
    void showShortcutsChanged(QT_STATE_CHANGED_T enabled);

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QPushButton openThemeButton;
    QLabel minPlayersForMultiColumnLayoutLabel;
    QLabel maxFontSizeForCardsLabel;
    QCheckBox showShortcutsCheckBox;
    QCheckBox displayCardNamesCheckBox;
    QCheckBox cardScalingCheckBox;
    QLabel verticalCardOverlapPercentLabel;
    QSpinBox verticalCardOverlapPercentBox;
    QCheckBox horizontalHandCheckBox;
    QCheckBox leftJustifiedHandCheckBox;
    QCheckBox invertVerticalCoordinateCheckBox;
    QGroupBox *themeGroupBox;
    QGroupBox *menuGroupBox;
    QGroupBox *cardsGroupBox;
    QGroupBox *handGroupBox;
    QGroupBox *tableGroupBox;
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
    QCheckBox playToStackCheckBox;
    QCheckBox annotateTokensCheckBox;
    QCheckBox useTearOffMenusCheckBox;
    QCheckBox tapAnimationCheckBox;
    QCheckBox openDeckInNewTabCheckBox;
    QGroupBox *generalGroupBox;
    QGroupBox *notificationsGroupBox;
    QGroupBox *animationGroupBox;
    QGroupBox *deckEditorGroupBox;

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
    QTreeWidget *shortcutsTable;
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
    void refreshShortcuts();
    void createShortcuts();
    void clearShortcuts();
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
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
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif
