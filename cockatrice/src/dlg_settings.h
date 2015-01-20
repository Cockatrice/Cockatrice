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
    QCheckBox picDownloadCheckBox;
    QCheckBox picDownloadHqCheckBox;
    QLabel languageLabel;
    QLabel pixmapCacheLabel;
    QLabel deckPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
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
    QCheckBox horizontalHandCheckBox;
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
    void soundPathClearButtonClicked();
    void soundPathButtonClicked();
signals:
    void soundPathChanged();
private:
    QCheckBox notificationsEnabledCheckBox;
    QCheckBox doubleClickToPlayCheckBox;
    QCheckBox playToStackCheckBox;
    QCheckBox tapAnimationCheckBox;
    QCheckBox soundEnabledCheckBox;
    QLabel soundPathLabel;
    QLineEdit *soundPathEdit;
    QGroupBox *generalGroupBox;
    QGroupBox *animationGroupBox;
    QGroupBox *soundGroupBox;
    QPushButton soundTestButton;
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
    void radioPriceTagSourceClicked(bool checked);
signals:
    void priceTagSourceChanged(int _priceTagSource);
private:
    QCheckBox priceTagsCheckBox;
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
private:
    QListWidget *messageList;
    QAction *aAdd;
    QAction *aRemove;
    QCheckBox chatMentionCheckBox;
    QGroupBox *chatGroupBox;
    QGroupBox *messageShortcuts;

    void storeSettings();
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
    QListWidgetItem *generalButton, *appearanceButton, *userInterfaceButton, *deckEditorButton, *messagesButton;
    void createIcons();
    void retranslateUi();
protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
};

#endif 
