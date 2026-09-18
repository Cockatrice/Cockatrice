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
    void actAdjustRateLimit();
    void resetDownloadedURLsButtonClicked();

private:
    QPushButton resetDownloadURLs;
    QLabel urlLinkLabel;
    QCheckBox picDownloadCheckBox;
    QListWidget *urlList;
    QAction *aAdd, *aEdit, *aRemove, *aRateLimit;
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

    /** @brief Adds a list item for the given URL, storing the raw URL alongside its displayed label. */
    QListWidgetItem *addUrlItem(const QString &url);

    /** @brief Returns the raw URL stored on a list item. */
    [[nodiscard]] QString urlForItem(const QListWidgetItem *item) const;

    /** @brief Returns the display label for a URL, including its current effective rate limit. */
    [[nodiscard]] QString urlLabel(const QString &url) const;

    /** @brief Refreshes the displayed label of every URL item after limits or settings change. */
    void refreshUrlItems();
};

#endif // COCKATRICE_DECK_EDITOR_SETTINGS_PAGE_H
