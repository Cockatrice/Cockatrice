#ifndef GAME_CARD_INFO_CONTROLLER_H
#define GAME_CARD_INFO_CONTROLLER_H

#include <QObject>
#include <QPointer>

class AbstractCardItem;
class CardInfoFrameWidget;
class GameScene;

/** Keeps the selected game card available in Card Info while other cards are previewed. */
class GameCardInfoController : public QObject
{
    Q_OBJECT

private:
    QPointer<GameScene> scene;
    QPointer<CardInfoFrameWidget> frame;
    QPointer<AbstractCardItem> selectedCard;
    QPointer<AbstractCardItem> hoveredCard;
    QPointer<AbstractCardItem> displayedCard;
    QMetaObject::Connection displayDestruction;
    quint64 displayRevision = 0;
    bool settingFrame = false;

    void updateSelection();
    void restoreSelection();
    void showCard(AbstractCardItem *card);

public:
    GameCardInfoController(GameScene *scene, CardInfoFrameWidget *frame, QObject *parent = nullptr);
    void registerCard(AbstractCardItem *card);
};

#endif
