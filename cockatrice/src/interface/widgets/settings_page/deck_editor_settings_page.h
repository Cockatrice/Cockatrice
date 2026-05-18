#ifndef COCKATRICE_DECK_EDITOR_SETTINGS_PAGE_H
#define COCKATRICE_DECK_EDITOR_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

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
    void resetDownloadedURLsButtonClicked();

private:
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
};

#endif // COCKATRICE_DECK_EDITOR_SETTINGS_PAGE_H
