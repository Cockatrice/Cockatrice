#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

class CardDatabase;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;
class QCloseEvent;
class QSpinBox;
class QRadioButton;
class QSpinBox;
class QSlider;
class QSpinBox;

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
    void tokenDatabasePathButtonClicked();
    void languageBoxChanged(int index);

private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
    QLineEdit *deckPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QSpinBox pixmapCacheEdit;
    QGroupBox *personalGroupBox;
    QGroupBox *pathsGroupBox;
    QComboBox languageBox;
    QCheckBox updateNotificationCheckBox;
    QCheckBox newVersionOracleCheckBox;
    QComboBox updateReleaseChannelBox;
    QLabel languageLabel;
    QLabel pixmapCacheLabel;
    QLabel deckPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLabel updateReleaseChannelLabel;
    QCheckBox showTipsOnStartup;
};

class AppearanceSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
private slots:
    void themeBoxChanged(int index);

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QLabel minPlayersForMultiColumnLayoutLabel;
    QLabel maxFontSizeForCardsLabel;
    QCheckBox displayCardNamesCheckBox;
    QCheckBox cardScalingCheckBox;
    QCheckBox horizontalHandCheckBox;
    QCheckBox leftJustifiedHandCheckBox;
    QCheckBox invertVerticalCoordinateCheckBox;
    QGroupBox *themeGroupBox;
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
    void setSpecNotificationEnabled(int);

private:
    QCheckBox notificationsEnabledCheckBox;
    QCheckBox specNotificationsEnabledCheckBox;
    QCheckBox doubleClickToPlayCheckBox;
    QCheckBox playToStackCheckBox;
    QCheckBox annotateTokensCheckBox;
    QCheckBox tapAnimationCheckBox;
    QGroupBox *generalGroupBox;
    QGroupBox *animationGroupBox;

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
    void updateTextColor(int value);
    void updateTextHighlightColor(int value);

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
    QGroupBox *messageShortcuts;
    QLineEdit *mentionColor;
    QLineEdit *highlightColor;
    QLineEdit *customAlertString;
    QLabel hexLabel;
    QLabel hexHighlightLabel;
    QLabel customAlertStringLabel;

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
        *soundButton;
    QListWidgetItem *shortcutsButton;
    void createIcons();
    void retranslateUi();

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif