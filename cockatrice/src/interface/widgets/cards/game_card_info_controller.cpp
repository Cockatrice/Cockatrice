#include "game_card_info_controller.h"

#include "../../../game_graphics/board/abstract_card_item.h"
#include "../../../game_graphics/board/card_item.h"
#include "../../../game_graphics/game_scene.h"
#include "card_info_frame_widget.h"

#include <QTimer>
#include <algorithm>

GameCardInfoController::GameCardInfoController(GameScene *_scene, CardInfoFrameWidget *_frame, QObject *parent)
    : QObject(parent), scene(_scene), frame(_frame)
{
    // Selection can change while a graphics item is being removed. Inspect the
    // resulting selection after that operation, not the partially destroyed item.
    connect(scene, &QGraphicsScene::selectionChanged, this, &GameCardInfoController::updateSelection,
            Qt::QueuedConnection);
    connect(scene, &GameScene::hoveredCardChanged, this, [this](CardItem *card) {
        hoveredCard = card;
        if (card) {
            showCard(card);
        } else {
            restoreSelection();
        }
    });
}

void GameCardInfoController::registerCard(AbstractCardItem *card)
{
    // Piles and pre-game deck views also request previews directly, without a
    // GameScene hover transition.
    connect(card, &AbstractCardItem::hovered, this, [this](AbstractCardItem *item) { showCard(item); });
    connect(card->getState(), &CardState::cardInfoChanged, this, [this, card] {
        if (displayedCard == card) {
            showCard(card);
        }
    });
    connect(card, &QObject::destroyed, this, [this] {
        QTimer::singleShot(0, this, [this] {
            updateSelection();
            if (!hoveredCard && selectedCard) {
                restoreSelection();
            } else if (!displayedCard && frame) {
                frame->clearCard();
            }
        });
    });
}

void GameCardInfoController::updateSelection()
{
    if (!scene) {
        return;
    }

    const auto cards = scene->selectedCards();
    // Keep a stable target while a multi-selection grows or its layout changes.
    if (selectedCard && cards.contains(qobject_cast<CardItem *>(selectedCard.data()))) {
        return;
    }

    selectedCard = nullptr;
    if (!cards.isEmpty()) {
        selectedCard = *std::min_element(cards.cbegin(), cards.cend(), [](CardItem *a, CardItem *b) {
            const auto aPos = a->scenePos();
            const auto bPos = b->scenePos();
            if (aPos.x() != bPos.x()) {
                return aPos.x() < bPos.x();
            }
            if (aPos.y() != bPos.y()) {
                return aPos.y() < bPos.y();
            }
            return a->getId() < b->getId();
        });
    }

    if (!hoveredCard && selectedCard) {
        showCard(selectedCard);
    }
}

void GameCardInfoController::restoreSelection()
{
    updateSelection();
    if (selectedCard) {
        showCard(selectedCard);
    }
}

void GameCardInfoController::showCard(AbstractCardItem *card)
{
    if (!card || !frame) {
        return;
    }

    displayedCard = card;
    const auto exactCard = card->getCard();
    // Repeated selection/hover signals must not reset the description scroll or
    // recreate the transformation button when the displayed printing is unchanged.
    if (frame->getCard() != exactCard) {
        frame->setCard(exactCard);
    }
}
