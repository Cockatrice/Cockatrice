#include "player_dialogs.h"

#include "../../client/settings/cache_settings.h"
#include "../../client/settings/card_counter_settings.h"
#include "../../interface/widgets/utility/get_text_with_max.h"
#include "../board/abstract_counter.h"
#include "../board/card_item.h"
#include "../dialogs/dlg_annotation.h"
#include "../dialogs/dlg_create_token.h"
#include "../dialogs/dlg_move_top_cards_until.h"
#include "../dialogs/dlg_roll_dice.h"
#include "../game_scene.h"
#include "../player/player_graphics_item.h"

#include <QChar>
#include <QInputDialog>
#include <QLineEdit>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/utility/string_limits.h>

PlayerDialogs::PlayerDialogs(PlayerGraphicsItem *_player, PlayerActions *_playerActions)
    : QObject(_player), player(_player), playerActions(_playerActions)
{
    connect(_playerActions, &PlayerActions::requestViewTopCardsDialog, this,
            &PlayerDialogs::onViewTopCardsDialogRequested);

    connect(_playerActions, &PlayerActions::requestViewBottomCardsDialog, this,
            &PlayerDialogs::onViewBottomCardsDialogRequested);

    connect(_playerActions, &PlayerActions::requestShuffleTopDialog, this, &PlayerDialogs::onShuffleTopDialogRequested);

    connect(_playerActions, &PlayerActions::requestShuffleBottomDialog, this,
            &PlayerDialogs::onShuffleBottomDialogRequested);

    connect(_playerActions, &PlayerActions::requestMulliganDialog, this, &PlayerDialogs::onMulliganDialogRequested);
    connect(_playerActions, &PlayerActions::startingHandSizeChosen, this, &PlayerDialogs::onStartingHandSizeChosen);
    connect(_playerActions, &PlayerActions::requestDrawCardsDialog, this, &PlayerDialogs::onDrawCardsDialogRequested);

    connect(_playerActions, &PlayerActions::requestMoveTopCardsToDialog, this,
            &PlayerDialogs::onMoveTopCardsToDialogRequested);

    connect(_playerActions, &PlayerActions::requestMoveTopCardsUntilDialog, this,
            &PlayerDialogs::onMoveTopCardsUntilDialogRequested);

    connect(_playerActions, &PlayerActions::requestMoveBottomCardsToDialog, this,
            &PlayerDialogs::onMoveBottomCardsToDialogRequested);

    connect(_playerActions, &PlayerActions::requestDrawBottomCardsDialog, this,
            &PlayerDialogs::onDrawBottomCardsDialogRequested);

    connect(_playerActions, &PlayerActions::requestRollDieDialog, this, &PlayerDialogs::onRollDieDialogRequested);

    connect(_playerActions, &PlayerActions::requestCreateTokenDialog, this,
            &PlayerDialogs::onCreateTokenDialogRequested);

    connect(_playerActions, &PlayerActions::requestCreateRelatedFromRelationDialog, this,
            &PlayerDialogs::onCreateRelatedFromRelationDialogRequested);

    connect(_playerActions, &PlayerActions::requestMoveCardXCardsFromTopDialog, this,
            &PlayerDialogs::onMoveCardXCardsFromTopDialogRequested);

    connect(_playerActions, &PlayerActions::requestSetPTDialog, this, &PlayerDialogs::onSetPTDialogRequested);

    connect(_playerActions, &PlayerActions::requestSetAnnotationDialog, this,
            &PlayerDialogs::onSetAnnotationDialogRequested);

    connect(_playerActions, &PlayerActions::requestSetCardCounterDialog, this,
            &PlayerDialogs::onSetCardCounterDialogRequested);
}

void PlayerDialogs::onViewTopCardsDialogRequested(int defaultNumberTopCards, int deckSize)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("View top cards of library"),
                                      tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberTopCards, 1,
                                      deckSize, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->actViewTopCards(number);
    }
}

void PlayerDialogs::onViewBottomCardsDialogRequested(int defaultNumberBottomCards, int deckSize)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("View bottom cards of library"),
                                      tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberBottomCards, 1,
                                      deckSize, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->actViewBottomCards(number);
    }
}

void PlayerDialogs::onShuffleTopDialogRequested(int defaultNumberTopCards, int maxCards)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Shuffle top cards of library"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberTopCards, 1,
                                      maxCards, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->actShuffleTop(number);
    }
}

void PlayerDialogs::onShuffleBottomDialogRequested(int defaultNumberBottomCards, int maxCards)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Shuffle bottom cards of library"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->actShuffleBottom(number);
    }
}

void PlayerDialogs::onMulliganDialogRequested(int startSize, int handSize, int deckSize)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Draw hand"),
                                      tr("Number of cards: (max. %1)").arg(deckSize) + '\n' +
                                          tr("0 and lower are in comparison to current hand size"),
                                      startSize, -handSize, deckSize, 1, &ok);

    if (ok && survivedDialog(self)) {
        playerActions->actMulligan(number);
    }
}

void PlayerDialogs::onStartingHandSizeChosen(int number)
{
    SettingsCache::instance().userInterface().setStartingHandSize(number);
}

void PlayerDialogs::onDrawCardsDialogRequested(int defaultNumberTopCards, int deckSize)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Draw cards"), tr("Number of cards: (max. %1)").arg(deckSize),
                                      defaultNumberTopCards, 1, deckSize, 1, &ok);

    if (ok && survivedDialog(self)) {
        playerActions->actDrawCards(number);
    }
}

void PlayerDialogs::onMoveTopCardsToDialogRequested(int defaultNumberTopCards,
                                                    int maxCards,
                                                    const QString &targetZone,
                                                    const QString &zoneDisplayName,
                                                    bool faceDown)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Move top cards to %1").arg(zoneDisplayName),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberTopCards, 1,
                                      maxCards, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->moveTopCardsTo(number, targetZone, faceDown);
    }
}

void PlayerDialogs::onMoveTopCardsUntilDialogRequested(MoveTopCardsUntilOptions options)
{
    QPointer<PlayerDialogs> self(this);
    DlgMoveTopCardsUntil dlg(dialogParent(), options);
    if (!dlg.exec() || !survivedDialog(self)) {
        return;
    }
    playerActions->moveTopCardsUntil(dlg.getExpr(), dlg.getOptions());
}

void PlayerDialogs::onMoveBottomCardsToDialogRequested(int defaultNumberBottomCards,
                                                       int maxCards,
                                                       const QString &targetZone,
                                                       const QString &zoneDisplayName,
                                                       bool faceDown)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number = QInputDialog::getInt(dialogParent(), tr("Move bottom cards to %1").arg(zoneDisplayName),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->moveBottomCardsTo(number, targetZone, faceDown);
    }
}

void PlayerDialogs::onDrawBottomCardsDialogRequested(int defaultNumberBottomCards, int maxCards)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number =
        QInputDialog::getInt(dialogParent(), tr("Draw bottom cards"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberBottomCards, 1, maxCards, 1, &ok);
    if (ok && survivedDialog(self)) {
        playerActions->actDrawBottomCards(number);
    }
}

void PlayerDialogs::onRollDieDialogRequested()
{
    QPointer<PlayerDialogs> self(this);
    DlgRollDice dlg(dialogParent());
    if (!dlg.exec() || !survivedDialog(self)) {
        return;
    }
    playerActions->actRollDie(dlg.getDieSideCount(), dlg.getDiceToRollCount());
}

void PlayerDialogs::onCreateRelatedFromRelationDialogRequested(const CardState *sourceCard,
                                                               const CardRelation &cardRelation)
{
    if (sourceCard == nullptr) {
        playerActions->setLastRelatedCreationSucceeded(false);
        return;
    }

    int variableCount = cardRelation.defaultCount;

    if (cardRelation.isVariableCount) {
        bool ok;

        emit requestDialogSemaphore(true);

        QPointer<PlayerDialogs> self(this);
        variableCount = QInputDialog::getInt(dialogParent(), tr("Create tokens"), tr("Number:"),
                                             cardRelation.defaultCount, 1, MAX_TOKENS_PER_DIALOG, 1, &ok);
        if (!survivedDialog(self)) {
            return;
        }

        emit requestDialogSemaphore(false);

        if (!ok) {
            playerActions->setLastRelatedCreationSucceeded(false); // cancelled
            return;
        }
    }

    const bool succeeded = playerActions->createRelatedFromRelation(sourceCard, cardRelation, variableCount);

    playerActions->setLastRelatedCreationSucceeded(succeeded);

    if (succeeded) {
        playerActions->onRelatedCardCreated(sourceCard, cardRelation); // only on confirmed success
    }
}

void PlayerDialogs::onCreateTokenDialogRequested(const QStringList &predefinedTokens)
{
    QPointer<PlayerDialogs> self(this);
    DlgCreateToken dlg(predefinedTokens, dialogParent());
    if (!dlg.exec() || !survivedDialog(self)) {
        return;
    }

    playerActions->actCreateToken(dlg.getTokenInfo());
}

void PlayerDialogs::onMoveCardXCardsFromTopDialogRequested(int defaultNumberTopCardsToPlaceBelow, int deckSize)
{
    bool ok;
    QPointer<PlayerDialogs> self(this);
    int number =
        QInputDialog::getInt(dialogParent(), tr("Place card X cards from top of library"),
                             tr("Which position should this card be placed:") + "\n" + tr("(max. %1)").arg(deckSize),
                             defaultNumberTopCardsToPlaceBelow, 1, deckSize, 1, &ok);
    number -= 1; // indexes start at 0

    if (ok && survivedDialog(self)) {
        playerActions->actMoveCardXCardsFromTop(player->getGameScene()->selectedCardStates(), number);
    }
}

void PlayerDialogs::onSetPTDialogRequested(const QString &oldPT)
{
    bool ok;
    auto cards = player->getGameScene()->selectedCardStates();
    emit requestDialogSemaphore(true);
    QPointer<PlayerDialogs> self(this);
    QString pt = getTextWithMax(dialogParent(), tr("Change power/toughness"), tr("Change stats to:"), QLineEdit::Normal,
                                oldPT, &ok);
    if (!survivedDialog(self)) {
        return;
    }
    emit requestDialogSemaphore(false);

    if (!ok || player->getLogic()->clearCardsToDelete()) {
        return;
    }

    playerActions->actSetPT(cards, pt);
}

void PlayerDialogs::onSetAnnotationDialogRequested(const QString &oldAnnotation)
{
    auto cards = player->getGameScene()->selectedCardStates();
    emit requestDialogSemaphore(true);
    AnnotationDialog *dialog = new AnnotationDialog(dialogParent());
    dialog->setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog->setWindowTitle(tr("Set annotation"));
    dialog->setLabelText(tr("Please enter the new annotation:"));
    dialog->setTextValue(oldAnnotation);
    QPointer<PlayerDialogs> self(this);
    bool ok = dialog->exec();
    if (!survivedDialog(self)) {
        return;
    }
    emit requestDialogSemaphore(false);
    if (!ok || player->getLogic()->clearCardsToDelete()) {
        return;
    }
    QString annotation = dialog->textValue().left(MAX_NAME_LENGTH);
    playerActions->actSetAnnotation(cards, annotation);
}

void PlayerDialogs::onSetCardCounterDialogRequested(int counterId, const QString &oldValueForDlg)
{
    auto cards = player->getGameScene()->selectedCardStates();
    emit requestDialogSemaphore(true);

    auto &cardCounterSettings = SettingsCache::instance().cardCounters();
    QString counterName = cardCounterSettings.displayName(counterId);

    AbstractCounterDialog dialog(counterName, oldValueForDlg, dialogParent());
    QPointer<PlayerDialogs> self(this);
    int ok = dialog.exec();
    if (!survivedDialog(self)) {
        return;
    }

    emit requestDialogSemaphore(false);
    if (!ok || player->getLogic()->clearCardsToDelete()) {
        return;
    }
    playerActions->actSetCardCounter(cards, counterId, dialog.textValue());
}