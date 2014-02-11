#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "cardzone.h"
#include "carditem.h"
#include "player.h"
#include "zoneviewzone.h"
#include "pb/command_move_card.pb.h"
#include "pb/serverinfo_user.pb.h"

CardZone::CardZone(Player *_p, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent, bool _isView)
    : AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), view(NULL), menu(NULL), doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable), isView(_isView)
{
    if (!isView)
        player->addZone(this);
}

CardZone::~CardZone()
{
    qDebug() << "CardZone destructor: " << name;
    delete view;
    clearContents();
}

void CardZone::retranslateUi()
{
    for (int i = 0; i < cards.size(); ++i)
        cards[i]->retranslateUi();
}

void CardZone::clearContents()
{
    for (int i = 0; i < cards.size(); i++) {
        // If an incorrectly implemented server doesn't return attached cards to whom they belong before dropping a player,
        // we have to return them to avoid a crash.
        const QList<CardItem *> &attachedCards = cards[i]->getAttachedCards();
        for (int j = 0; j < attachedCards.size(); ++j)
            attachedCards[j]->setParentItem(attachedCards[j]->getZone());
        
        player->deleteCard(cards.at(i));
    }
    cards.clear();
    emit cardCountChanged();
}

QString CardZone::getTranslatedName(bool hisOwn, GrammaticalCase gc) const
{
    QString ownerName = player->getName();
    bool female = player->getUserInfo()->gender() == ServerInfo_User::Female;
    if (name == "hand")
        return female
            ? (hisOwn
                ? tr("her hand", "nominative, female owner")
                : tr("%1's hand", "nominative, female owner").arg(ownerName)
            ) : (hisOwn
                ? tr("his hand", "nominative, male owner")
                : tr("%1's hand", "nominative, male owner").arg(ownerName)
            );
    else if (name == "deck")
        switch (gc) {
        case CaseLookAtZone:
            return female
                ? (hisOwn
                    ? tr("her library", "look at zone, female owner")
                    : tr("%1's library", "look at zone, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his library", "look at zone, male owner")
                    : tr("%1's library", "look at zone, male owner").arg(ownerName)
                );
        case CaseTopCardsOfZone:
            return female
                ? (hisOwn
                    ? tr("of her library", "top cards of zone, female owner")
                    : tr("of %1's library", "top cards of zone, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("of his library", "top cards of zone, male owner")
                    : tr("of %1's library", "top cards of zone, male owner").arg(ownerName)
                );
        case CaseRevealZone:
            return female
                ? (hisOwn
                    ? tr("her library", "reveal zone, female owner")
                    : tr("%1's library", "reveal zone, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his library", "reveal zone, male owner")
                    : tr("%1's library", "reveal zone, male owner").arg(ownerName)
                );
        case CaseShuffleZone:
            return female
                ? (hisOwn
                    ? tr("her library", "shuffle, female owner")
                    : tr("%1's library", "shuffle, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his library", "shuffle, male owner")
                    : tr("%1's library", "shuffle, male owner").arg(ownerName)
                );
        default:
            return female
                ? (hisOwn
                    ? tr("her library", "nominative, female owner")
                    : tr("%1's library", "nominative, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his library", "nominative, male owner")
                    : tr("%1's library", "nominative, male owner").arg(ownerName)
                );
        }
    else if (name == "grave")
        return female
            ? (hisOwn
                ? tr("her graveyard", "nominative, female owner")
                : tr("%1's graveyard", "nominative, female owner").arg(ownerName)
            ) : (hisOwn
                ? tr("his graveyard", "nominative, male owner")
                : tr("%1's graveyard", "nominative, male owner").arg(ownerName)
            );
    else if (name == "rfg")
        return female
            ? (hisOwn
                ? tr("her exile", "nominative, female owner")
                : tr("%1's exile", "nominative, female owner").arg(ownerName)
            ) : (hisOwn
                ? tr("his exile", "nominative, male owner")
                : tr("%1's exile", "nominative, male owner").arg(ownerName)
            );
    else if (name == "sb")
        switch (gc) {
        case CaseLookAtZone:
            return female
                ? (hisOwn
                    ? tr("her sideboard", "look at zone, female owner")
                    : tr("%1's sideboard", "look at zone, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his sideboard", "look at zone, male owner")
                    : tr("%1's sideboard", "look at zone, male owner").arg(ownerName)
                );
        case CaseNominative:
            return female
                ? (hisOwn
                    ? tr("her sideboard", "nominative, female owner")
                    : tr("%1's sideboard", "nominative, female owner").arg(ownerName)
                ) : (hisOwn
                    ? tr("his sideboard", "nominative, male owner")
                    : tr("%1's sideboard", "nominative, male owner").arg(ownerName)
                );
        default: break;
        }
    return QString();
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
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

void CardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
    if (view)
        if ((x <= view->getCards().size()) || (view->getNumberCards() == -1))
            view->addCard(new CardItem(player, card->getName(), card->getId()), reorganize, x, y);

    card->setZone(this);
    addCardImpl(card, x, y);

    if (reorganize)
        reorganizeCards();
    
    emit cardCountChanged();
}

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
    CardItem *c = cards.findCard(cardId, false);
    if (!c) {
        qDebug() << "CardZone::getCard: card id=" << cardId << "not found";
        return 0;
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

CardItem *CardZone::takeCard(int position, int cardId, bool /*canResize*/)
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
        return 0;

    CardItem *c = cards.takeAt(position);

    if (view)
        view->removeCard(position);

    c->setId(cardId);

    reorganizeCards();
    emit cardCountChanged();
    return c;
}

void CardZone::removeCard(CardItem *card)
{
    cards.removeAt(cards.indexOf(card));
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
