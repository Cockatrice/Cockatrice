#include "card_zone.h"

#include "../board/card_item.h"
#include "../cards/card_database_manager.h"
#include "../player/player.h"
#include "pb/command_move_card.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pile_zone.h"
#include "view_zone.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

/**
 * @param _p the player that the zone belongs to
 * @param _name internal name of the zone
 * @param _isShufflable whether it makes sense to shuffle this zone by default after viewing it
 * @param _contentsKnown whether the cards in the zone are known to the client
 * @param parent the parent graphics object.
 */
CardZone::CardZone(Player *_p,
                   const QString &_name,
                   bool _hasCardAttr,
                   bool _isShufflable,
                   bool _contentsKnown,
                   QGraphicsItem *parent)
    : AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), views{}, menu(nullptr),
      doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable)
{
    // If we join a game before the card db finishes loading, the cards might have the wrong printings.
    // Force refresh all cards in the zone when db finishes loading to fix that.
    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            &CardZone::refreshCardInfos);
}

void CardZone::retranslateUi()
{
    for (int i = 0; i < cards.size(); ++i)
        cards[i]->retranslateUi();
}

void CardZone::clearContents()
{
    for (int i = 0; i < cards.size(); i++) {
        // If an incorrectly implemented server doesn't return attached cards to whom they belong before dropping a
        // player, we have to return them to avoid a crash.
        const QList<CardItem *> &attachedCards = cards[i]->getAttachedCards();
        for (auto attachedCard : attachedCards)
            attachedCard->setParentItem(attachedCard->getZone());

        player->deleteCard(cards.at(i));
    }
    cards.clear();
    emit cardCountChanged();
}

QString CardZone::getTranslatedName(bool theirOwn, GrammaticalCase gc) const
{
    QString ownerName = player->getName();
    if (name == "hand")
        return (theirOwn ? tr("their hand", "nominative") : tr("%1's hand", "nominative").arg(ownerName));
    else if (name == "deck")
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
    else if (name == "grave")
        return (theirOwn ? tr("their graveyard", "nominative") : tr("%1's graveyard", "nominative").arg(ownerName));
    else if (name == "rfg")
        return (theirOwn ? tr("their exile", "nominative") : tr("%1's exile", "nominative").arg(ownerName));
    else if (name == "sb")
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
    else {
        return (theirOwn ? tr("their custom zone '%1'", "nominative").arg(name)
                         : tr("%1's custom zone '%2'", "nominative").arg(ownerName).arg(name));
    }
    return QString();
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    if (doubleClickAction)
        doubleClickAction->trigger();
}

bool CardZone::showContextMenu(const QPoint &screenPos)
{
    if (menu) {
        menu->exec(screenPos);
        return true;
    }
    return false;
}

void CardZone::refreshCardInfos()
{
    for (const auto &cardItem : cards) {
        cardItem->refreshCardInfo();
    }
}

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

void CardZone::addCard(CardItem *card, const bool reorganize, const int x, const int y)
{
    if (!card) {
        qCWarning(CardZoneLog) << "CardZone::addCard() card is null; this shouldn't normally happen";
        return;
    }

    for (auto *view : views) {
        if (view->prepareAddCard(x)) {
            view->addCard(new CardItem(player, nullptr, card->getName(), card->getProviderId(), card->getId()),
                          reorganize, x, y);
        }
    }

    card->setZone(this);
    addCardImpl(card, x, y);

    if (reorganize)
        reorganizeCards();

    emit cardCountChanged();
}

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
    CardItem *c = cards.findCard(cardId);
    if (!c) {
        qCWarning(CardZoneLog) << "CardZone::getCard: card id=" << cardId << "not found";
        return nullptr;
    }
    // If the card's id is -1, this zone is invisible,
    // so we need to give the card an id and a name as it comes out.
    // It can be assumed that in an invisible zone, all cards are equal.
    if ((c->getId() == -1) || (c->getName().isEmpty())) {
        c->setId(cardId);
        c->setName(cardName);
    }
    return c;
}

CardItem *CardZone::takeCard(int position, int cardId, bool toNewZone)
{
    if (position == -1) {
        // position == -1 means either that the zone is indexed by card id
        // or that it doesn't matter which card you take.
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

    for (auto *view : views) {
        view->removeCard(position, toNewZone);
    }

    CardItem *c = cards.takeAt(position);

    c->setId(cardId);

    reorganizeCards();
    emit cardCountChanged();
    return c;
}

void CardZone::removeCard(CardItem *card)
{
    if (!card) {
        qCWarning(CardZoneLog) << "CardZone::removeCard: card is null, this shouldn't normally happen";
        return;
    }

    cards.removeOne(card);
    reorganizeCards();
    emit cardCountChanged();
    player->deleteCard(card);
}

void CardZone::moveAllToZone()
{
    QList<QVariant> data = static_cast<QAction *>(sender())->data().toList();
    QString targetZone = data[0].toString();
    int targetX = data[1].toInt();

    Command_MoveCard cmd;
    cmd.set_start_zone(getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(targetZone.toStdString());
    cmd.set_x(targetX);

    for (int i = 0; i < cards.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(cards[i]->getId());

    player->sendGameCommand(cmd);
}

QPointF CardZone::closestGridPoint(const QPointF &point)
{
    return point;
}
