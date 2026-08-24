#ifndef COCKATRICE_GENERAL_SETTINGS_PAGE_H
#define COCKATRICE_GENERAL_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSpinBox>

inline Q_LOGGING_CATEGORY(GeneralSettingsPageLog, "general_settings_page");

class GeneralSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    GeneralSettingsPage();
    void retranslateUi() override;

    static QStringList findQmFiles();
    static QString languageName(const QString &lang);

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
    void updateStartupServerControlsVisibility();

private:
    QGroupBox *languageGroupBox;
    QGroupBox *versionGroupBox;
    QGroupBox *cardDatabaseGroupBox;
    QGroupBox *startupGroupBox;
    QGroupBox *pathsGroupBox;

    QLabel languageLabel;
    QComboBox languageBox;
    QLabel advertiseTranslationPageLabel;

    QComboBox updateReleaseChannelBox;
    QCheckBox startupUpdateCheckCheckBox;
    QCheckBox updateNotificationCheckBox;
    QCheckBox newVersionOracleCheckBox;

    QLabel startupCardUpdateCheckBehaviorLabel;
    QComboBox startupCardUpdateCheckBehaviorSelector;
    QLabel cardUpdateCheckIntervalLabel;
    QSpinBox cardUpdateCheckIntervalSpinBox;
    QLabel lastCardUpdateCheckDateLabel;

    QLabel updateReleaseChannelLabel;

    QCheckBox showTipsOnStartup;
    QLabel startupTabLabel;
    QComboBox startupTabSelector;
    QLabel startupServerLabel;
    QComboBox startupServerSelector;
    QLabel startupRoomLabel;
    QLineEdit *startupRoomNameEdit;

    QLabel deckPathLabel;
    QLabel filtersPathLabel;
    QLabel replaysPathLabel;
    QLabel picsPathLabel;
    QLabel cardDatabasePathLabel;
    QLabel customCardDatabasePathLabel;
    QLabel tokenDatabasePathLabel;
    QLineEdit *deckPathEdit;
    QLineEdit *filtersPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *customCardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QPushButton *resetAllPathsButton;
    QLabel *allPathsResetLabel;
};

#endif // COCKATRICE_GENERAL_SETTINGS_PAGE_H
