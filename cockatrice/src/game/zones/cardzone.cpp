#include "cardzone.h"

#include "../cards/carditem.h"
#include "pb/command_move_card.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "../../player/player.h"
#include "zoneviewzone.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

CardZone::CardZone(Player *_p,
                   const QString &_name,
                   bool _hasCardAttr,
                   bool _isShufflable,
                   bool _contentsKnown,
                   QGraphicsItem *parent,
                   bool _isView)
    : AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), views{}, menu(nullptr),
      doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable), isView(_isView)
{
    if (!isView)
        player->addZone(this);
}

CardZone::~CardZone()
{
    qDebug() << "CardZone destructor: " << name;
    for (auto *view : views) {
        if (view != nullptr) {
            view->deleteLater();
        }
    }
    clearContents();
}

/**
 * @brief Retranslates the user interface for all cards in the zone.
 *
 * This function calls the `retranslateUi()` method on each card in the card zone,
 * ensuring that all displayed text is updated to reflect any changes in language or localization.
 */
void CardZone::retranslateUi()
{
    for (auto & card : cards)
        card->retranslateUi();
}

/**
 * @brief Clears the contents of the card zone and safely handles attached cards.
 *
 * This function removes all cards from the card zone. If the server does not properly return attached cards
 * to their respective owners before removing a player, this function ensures that attached cards are returned
 * to their original zones to avoid crashes. Each card is deleted from the player's collection, and the internal
 * card list is cleared. Finally, a signal is emitted to notify that the card count has changed.
 *
 * @emit cardCountChanged() Signal emitted after the card list has been cleared.
 */
void CardZone::clearContents()
{
    for (auto & card : cards) {
        const QList<CardItem *> &attachedCards = card->getAttachedCards();
        for (const auto attachedCard : attachedCards)
            attachedCard->setParentItem(attachedCard->getZone());

        player->deleteCard(card);
    }
    cards.clear();
    emit cardCountChanged();
}

/**
 * @brief Returns the translated name of the card zone based on ownership and grammatical case.
 *
 * Retrieves a localized zone name (e.g., hand, deck, graveyard) for the player or opponent,
 * adjusting for grammatical case (e.g., nominative, look at zone, shuffle).
 *
 * @param theirOwn True if the zone belongs to the player, false for the opponent's.
 * @param gc The grammatical case for translation.
 * @return QString The localized name of the zone, or an empty string if unrecognized.
 */
QString CardZone::getTranslatedName(bool theirOwn, GrammaticalCase gc) const
{
    const QString ownerName = player->getName();
    switch ( name ) {
        case "hand":
            return (theirOwn ? tr("their hand", "nominative") : tr("%1's hand", "nominative").arg(ownerName));
        case "deck":
            switch (gc) {
                case CaseLookAtZone:
                    return (theirOwn ? tr("their library", "look at zone")
                                     : tr("%1's library", "look at zone").arg(ownerName));
                case CaseTopCardsOfZone:
                    return (theirOwn ? tr("of their library", "top cards of zone,")
                                     : tr("of %1's library", "top cards of zone").arg(ownerName));
                case CaseRevealZone:
                    return (theirOwn ? tr("their library", "reveal zone")
                                     : tr("%1's library", "reveal zone").arg(ownerName));
                case CaseShuffleZone:
                    return (theirOwn ? tr("their library", "shuffle") : tr("%1's library", "shuffle").arg(ownerName));
                default:
                    return (theirOwn ? tr("their library", "nominative") : tr("%1's library", "nominative").arg(ownerName));
            }
        case "grave":
            return (theirOwn ? tr("their graveyard", "nominative") : tr("%1's graveyard", "nominative").arg(ownerName));
        case "rfg":
            return (theirOwn ? tr("their exile", "nominative") : tr("%1's exile", "nominative").arg(ownerName));
        case "sb":
            switch (gc) {
                case CaseLookAtZone:
                    return (theirOwn ? tr("their sideboard", "look at zone")
                                     : tr("%1's sideboard", "look at zone").arg(ownerName));
                case CaseNominative:
                    return (theirOwn ? tr("their sideboard", "nominative")
                                     : tr("%1's sideboard", "nominative").arg(ownerName));
                default:
                    break;
            }
        default:
            return {};
    }
}

/**
 * @brief Handles double-click events on the card zone.
 *
 * This function is triggered when a mouse double-click event occurs within the card zone.
 * If a `doubleClickAction` is set, it will trigger the associated action.
 *
 * @param event The double-click event (unused in this implementation).
 */
void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    if (doubleClickAction)
        doubleClickAction->trigger();
}

/**
 * @brief Displays the context menu at the specified screen position.
 *
 * If a context menu is available, it will be executed at the provided screen position.
 * The function does not modify the state of the object and can be marked as `const`.
 *
 * @param screenPos The position on the screen where the context menu should appear.
 * @return bool Returns true if the menu was shown, false if no menu is available.
 */
bool CardZone::showContextMenu(const QPoint &screenPos)
{
    if (menu) {
        menu->exec(screenPos);
        return true;
    }
    return false;
}

/**
 * @brief Handles mouse press events in the card zone.
 *
 * This function processes mouse press events, specifically checking for a right-click.
 * If the right mouse button is pressed, it attempts to show a context menu at the
 * mouse position. If the menu is displayed successfully, the event is accepted;
 * otherwise, it is ignored. Any other mouse button press is also ignored.
 *
 * @param event The mouse press event containing information about the button pressed.
 */
void CardZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (showContextMenu(event->screenPos()))
            event->accept();
        else
            event->ignore();
    } else
        event->ignore();
}

/**
 * @brief Adds a card to the card zone and updates views.
 *
 * This function adds a `CardItem` to the `CardZone`, checking if it can be added
 * based on the current number of cards in the views. If `reorganize` is true,
 * it reorganizes the cards after adding. The card's zone is set to this instance.
 *
 * @param card Pointer to the `CardItem` to be added (must not be null).
 * @param reorganize If true, reorganizes cards after addition.
 * @param x X-coordinate for positioning the card.
 * @param y Y-coordinate for positioning the card.
 *
 * @post Emits `cardCountChanged` signal to notify about the updated card count.
 */
void CardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
    for (auto *view : views) {
        if ((x <= view->getCards().size()) || (view->getNumberCards() == -1)) {
            view->addCard(new CardItem(player, card->getName(), card->getId()), reorganize, x, y);
        }
    }

    card->setZone(this);
    addCardImpl(card, x, y);

    if (reorganize)
        reorganizeCards();

    emit cardCountChanged();
}

/**
 * @brief Retrieves a card from the card zone by its ID and updates its properties if needed.
 *
 * This function looks for a `CardItem` in the `CardZone` based on the provided
 * card ID. If the card is not found, it logs a debug message and returns a null pointer.
 * If the card has an ID of -1, this current CardZone is invisible so we need to give
 * the card an id and a name as it comes out of this zone.
 * It can be assumed that in an invisible zone, all cards are equal.
 *
 * @param cardId The ID of the card to retrieve.
 * @param cardName The name to assign to the card if it is in an invisible zone.
 * @return A pointer to the `CardItem` if found; otherwise, returns nullptr.
 *
 * @note This method assumes that in an invisible zone, all cards are treated equally.
 */
CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
    CardItem *c = cards.findCard(cardId, false);
    if (!c) {
        qDebug() << "CardZone::getCard: card id=" << cardId << "not found";
        return nullptr;
    }

    if ((c->getId() == -1) || (c->getName().isEmpty())) {
        c->setId(cardId);
        c->setName(cardName);
    }
    return c;
}

/**
 * @brief Removes a card from the card zone at the specified position.
 *
 * This function retrieves and removes a `CardItem` from the `CardZone`
 * at the given index. If `position` is -1, it searches for the card
 * by `cardId`, meaning either the zone is indexed by card ID or it
 * doesn't matter which card is taken. If not found, it defaults to the
 * first card. After removal, it updates associated views and reorganizes
 * the remaining cards.
 *
 * @param position The index of the card to remove. If -1, the function
 *        searches for the card by `cardId`.
 * @param cardId The ID of the card to be taken.
 * @param canResize (Unused) A flag indicating if the zone can resize,
 *        included for compatibility.
 *
 * @return A pointer to the removed `CardItem`, or nullptr if the
 *         position is invalid or no card was found.
 *
 * @post Emits `cardCountChanged` to notify observers of the updated card count.
 */
CardItem *CardZone::takeCard(int position, int cardId, bool /*canResize*/)
{
    if (position == -1) {
        for (int i = 0; i < cards.size(); ++i)
            if (cards[i]->getId() == cardId) {
                position = i;
                break;
            }
        if (position == -1)
            position = 0;
    }
    if (position >= cards.size())
        return nullptr;

    CardItem *c = cards.takeAt(position);

    for (auto *view : views) {
        view->removeCard(position);
    }

    c->setId(cardId);

    reorganizeCards();
    emit cardCountChanged();
    return c;
}

/**
 * @brief Removes a specified card from the card zone.
 *
 * This function removes the given `CardItem` from the `CardZone`.
 * After removal, it reorganizes the remaining cards and emits a
 * signal to notify observers of the updated card count. Finally,
 * it calls the player's method to delete the card from memory.
 *
 * @param card Pointer to the `CardItem` to be removed.
 *        Must not be null.
 *
 * @post Emits `cardCountChanged` to notify observers of the updated card count.
 */
void CardZone::removeCard(CardItem *card)
{
    cards.removeOne(card);
    reorganizeCards();
    emit cardCountChanged();
    player->deleteCard(card);
}

/**
 * @brief Moves all cards from the current zone to a target zone.
 *
 * This function retrieves the target zone and coordinates from the sender's
 * action data, constructs a `Command_MoveCard` command, and populates it
 * with all cards currently in the `CardZone`. It then sends the command to
 * the player to execute the move.
 *
 * The expected action data structure contains:
 * - The target zone as a string.
 * - The target x-coordinate as an integer.
 *
 * @post Sends a `Command_MoveCard` command to the player to move all cards
 *       to the specified target zone.
 */
void CardZone::moveAllToZone()
{
    QList<QVariant> data = dynamic_cast<QAction *>(sender())->data().toList();
    QString targetZone = data[0].toString();
    int targetX = data[1].toInt();

    Command_MoveCard cmd;
    cmd.set_start_zone(getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(targetZone.toStdString());
    cmd.set_x(targetX);

    for (auto & card : cards)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(card->getId());

    player->sendGameCommand(cmd);
}

/**
 * @brief Returns the closest grid point to the specified point.
 *
 * This function takes a point as input and currently returns it unchanged.
 *
 * @param point The input point for which the closest grid point is to be determined.
 *
 * @return The input point, unchanged.
 */
QPointF CardZone::closestGridPoint(const QPointF &point)
{
    return point;
}
