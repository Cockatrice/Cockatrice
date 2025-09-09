#include "player_info.h"

PlayerInfo::PlayerInfo(const ServerInfo_User &info, int _id, bool _local, bool _judge)
    : id(_id), local(_local), judge(_judge), handVisible(false), conceded(false), zoneId(0), deck(nullptr)
{
    userInfo = new ServerInfo_User;
    userInfo->CopyFrom(info);
}

void PlayerInfo::setDeck(const DeckLoader &_deck)
{
    deck = new DeckLoader(_deck);
    // TODO: deal with this
    /*aOpenDeckInDeckEditor->setEnabled(deck);

    createPredefinedTokenMenu->clear();
    createPredefinedTokenMenu->setEnabled(false);
    predefinedTokens.clear();
    InnerDecklistNode *tokenZone = dynamic_cast<InnerDecklistNode *>(deck->getRoot()->findChild(DECK_ZONE_TOKENS));

    if (tokenZone) {
        if (tokenZone->size() > 0)
            createPredefinedTokenMenu->setEnabled(true);

        for (int i = 0; i < tokenZone->size(); ++i) {
            const QString tokenName = tokenZone->at(i)->getName();
            predefinedTokens.append(tokenName);
            QAction *a = createPredefinedTokenMenu->addAction(tokenName);
            if (i < 10) {
                a->setShortcut(QKeySequence("Alt+" + QString::number((i + 1) % 10)));
            }
            connect(a, &QAction::triggered, this, &Player::actCreatePredefinedToken);
        }
    }*/
}

void PlayerInfo::setConceded(bool _conceded)
{
    conceded = _conceded;
    // TODO: deal with this
    /*setVisible(!conceded);
    if (conceded) {
        clear();
    }
    emit playerCountChanged();*/
}

void PlayerInfo::setZoneId(int _zoneId)
{
    // TODO: deal with this
    zoneId = _zoneId;
    // playerArea->setPlayerZoneId(_zoneId);
}