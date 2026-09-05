/**
 * @file tab_public_decks.h
 * @ingroup Tabs
 */

#ifndef TAB_PUBLIC_DECKS_H
#define TAB_PUBLIC_DECKS_H

#include "tab.h"

class AbstractClient;
class CommandContainer;
class DeckPreviewColorIdentityFilterWidget;
class FlowWidget;
class PublicDeckPreviewWidget;
class PublicDecksQuickSettingsWidget;
class QLabel;
class QToolButton;
class RemotePublicDecksModel;
class Response;
class VisualDeckStorageSearchWidget;
class VisualDeckStorageTagFilterWidget;

/**
 * @brief A visual grid of the public decks published by another user.
 *
 * The grid is rendered from the preview metadata the server stores for the
 * decks, so browsing costs no downloads; the deck list is fetched via
 * Command_DeckDownloadPublic only when the user opens a deck. Multiple users
 * can be browsed simultaneously; each gets its own tab.
 */
class TabPublicDecks final : public Tab
{
    Q_OBJECT

public:
    TabPublicDecks(TabSupervisor *tabSupervisor, AbstractClient *client, const QString &userName);

    [[nodiscard]] QString getTabText() const override;
    void retranslateUi() override;
    bool closeRequest() override;

    [[nodiscard]] QString getUserName() const
    {
        return userName;
    }

signals:
    void closing(TabPublicDecks *tab);

private slots:
    void openDeck(int deckId);
    void openDeckFinished(const Response &response, const CommandContainer &commandContainer);
    void updateColorFilter();
    void updateTagFilter();
    void updateCardSize(int scale);
    void updateTagsVisibility(bool visible);
    void updateLoadingState(bool loading);

private:
    void rebuildGrid();
    void applyCardSize(int scale);

    AbstractClient *client;
    QString userName;
    RemotePublicDecksModel *model;
    FlowWidget *flowWidget;
    VisualDeckStorageSearchWidget *searchWidget;
    DeckPreviewColorIdentityFilterWidget *colorIdentityFilter;
    VisualDeckStorageTagFilterWidget *tagFilterWidget;
    QToolButton *refreshButton;
    PublicDecksQuickSettingsWidget *quickSettingsWidget;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QLabel *emptyLabel;
    int cardSize = 100;
};

#endif // TAB_PUBLIC_DECKS_H
