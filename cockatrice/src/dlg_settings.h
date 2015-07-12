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
private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
    QLineEdit *deckPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QLineEdit *highQualityURLEdit;
    QSpinBox pixmapCacheEdit;
    QGroupBox *personalGroupBox; 
    QGroupBox *pathsGroupBox;
    QComboBox languageBox;
    QCheckBox picDownloadCheckBox;
    QCheckBox picDownloadHqCheckBox;
    QLabel languageLabel;
    QLabel pixmapCacheLabel;
    QLabel deckPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLabel highQualityURLLabel;
    QLabel highQualityURLLinkLabel;
    QPushButton clearDownloadedPicsButton;
};

class AppearanceSettingsPage : public AbstractSettingsPage {
    Q_OBJECT
private slots:
    void handBgClearButtonClicked();
    void handBgButtonClicked();
    void stackBgClearButtonClicked();
    void stackBgButtonClicked();
    void tableBgClearButtonClicked();
    void tableBgButtonClicked();
    void playerAreaBgClearButtonClicked();
    void playerAreaBgButtonClicked();
    void cardBackPicturePathClearButtonClicked();
    void cardBackPicturePathButtonClicked();
signals:
    void handBgChanged(const QString &path);
    void stackBgChanged(const QString &path);
    void tableBgChanged(const QString &path);
    void playerAreaBgChanged(const QString &path);
    void cardBackPicturePathChanged(const QString &path);
private:
    QLabel handBgLabel;
    QLabel stackBgLabel;
    QLabel tableBgLabel;
    QLabel playerAreaBgLabel;
    QLabel cardBackPicturePathLabel;
    QLabel minPlayersForMultiColumnLayoutLabel;
    QLineEdit *handBgEdit;
    QLineEdit *stackBgEdit;
    QLineEdit *tableBgEdit;
    QLineEdit *playerAreaBgEdit;
    QLineEdit *cardBackPicturePathEdit;
    QCheckBox displayCardNamesCheckBox;
    QCheckBox cardScalingCheckBox;
    QCheckBox horizontalHandCheckBox;
    QCheckBox leftJustifiedHandCheckBox;
    QCheckBox invertVerticalCoordinateCheckBox;
    QGroupBox *zoneBgGroupBox;
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
    QCheckBox invertMentionForeground;
    QCheckBox invertHighlightForeground;
    QCheckBox ignoreUnregUsersMainChat;
    QCheckBox ignoreUnregUserMessages;
    QCheckBox messagePopups;
    QCheckBox mentionPopups;
    QGroupBox *chatGroupBox;
    QGroupBox *highlightGroupBox;
    QGroupBox *messageShortcuts;
    QLineEdit *mentionColor;
    QLineEdit *customAlertString;
    QLineEdit *highlightColor;
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
    QGroupBox *soundGroupBox;
    QPushButton soundTestButton;
    QCheckBox soundEnabledCheckBox;
    QLabel soundPathLabel;
    QLineEdit *soundPathEdit;
    QLabel masterVolumeLabel;
    QSlider *masterVolumeSlider;
    QSpinBox *masterVolumeSpinBox;
signals:
    void soundPathChanged();
private slots:
    void masterVolumeChanged(int value);
    void soundPathClearButtonClicked();
    void soundPathButtonClicked();
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
    void createIcons();
    void retranslateUi();
protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
};

#endif 
