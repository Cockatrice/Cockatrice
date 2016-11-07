#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QComboBox>
#include <QCheckBox>
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

class AbstractSettingsPage : public QWidget {
public:
    virtual void retranslateUi() = 0;
};

class GeneralSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
public:
    GeneralSettingsPage();
    void retranslateUi();
private slots:
    void deckPathButtonClicked();
    void replaysPathButtonClicked();
    void picsPathButtonClicked();
    void clearDownloadedPicsButtonClicked();
    void cardDatabasePathButtonClicked();
    void tokenDatabasePathButtonClicked();
    void languageBoxChanged(int index);
    void setEnabledStatus(bool);
    void defaultUrlRestoreButtonClicked();
    void fallbackUrlRestoreButtonClicked();
private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
    QLineEdit *deckPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QLineEdit *defaultUrlEdit;
    QLineEdit *fallbackUrlEdit;
    QSpinBox pixmapCacheEdit;
    QGroupBox *personalGroupBox; 
    QGroupBox *pathsGroupBox;
    QComboBox languageBox;
    QCheckBox picDownloadCheckBox;
    QCheckBox updateNotificationCheckBox;
    QLabel languageLabel;
    QLabel pixmapCacheLabel;
    QLabel deckPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLabel defaultUrlLabel;
    QLabel fallbackUrlLabel;
    QLabel urlLinkLabel;
    QPushButton clearDownloadedPicsButton;
    QPushButton defaultUrlRestoreButton;
    QPushButton fallbackUrlRestoreButton;
};

class AppearanceSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
private slots:
    void themeBoxChanged(int index);
private:
    QLabel themeLabel;
    QComboBox themeBox;
    QLabel minPlayersForMultiColumnLayoutLabel;
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
public:
    AppearanceSettingsPage();
    void retranslateUi();
};

class UserInterfaceSettingsPage : public AbstractSettingsPage {
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
    void retranslateUi();
};

class DeckEditorSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
public:
    DeckEditorSettingsPage();
    void retranslateUi();
private slots:
    //void radioPriceTagSourceClicked(bool checked);
signals:
    //void priceTagSourceChanged(int _priceTagSource);
private:
    //QCheckBox priceTagsCheckBox;
    QGroupBox *generalGroupBox;
};

class MessagesSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
public:
    MessagesSettingsPage();
    void retranslateUi();
private slots:
    void actAdd();
    void actRemove();
    void updateColor(const QString &value);
    void updateHighlightColor(const QString &value);
    void updateTextColor(int value);
    void updateTextHighlightColor(int value);
private:
    QListWidget *messageList;
    QAction *aAdd;
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

class SoundSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
public:
    SoundSettingsPage();
    void retranslateUi();
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

class DlgSettings : public QDialog {
    Q_OBJECT
public:
    DlgSettings(QWidget *parent = 0);
    void setTab(int index);
private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void updateLanguage();
private:
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    QListWidgetItem *generalButton, *appearanceButton, *userInterfaceButton, *deckEditorButton, *messagesButton, *soundButton;
    QListWidgetItem *shortcutsButton;
    void createIcons();
    void retranslateUi();
protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
};

#endif 
