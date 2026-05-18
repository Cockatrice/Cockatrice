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

    QGroupBox *languageGroupBox;
    QGroupBox *versionGroupBox;
    QGroupBox *cardDatabaseGroupBox;
    QGroupBox *startupGroupBox;
    QGroupBox *pathsGroupBox;

    QLineEdit *deckPathEdit;
    QLineEdit *filtersPathEdit;
    QLineEdit *replaysPathEdit;
    QLineEdit *picsPathEdit;
    QLineEdit *cardDatabasePathEdit;
    QLineEdit *customCardDatabasePathEdit;
    QLineEdit *tokenDatabasePathEdit;
    QPushButton *resetAllPathsButton;
    QLabel *allPathsResetLabel;
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

#endif // COCKATRICE_GENERAL_SETTINGS_PAGE_H
