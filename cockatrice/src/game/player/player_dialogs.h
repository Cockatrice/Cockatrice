#ifndef COCKATRICE_PLAYER_DIALOGS_H
#define COCKATRICE_PLAYER_DIALOGS_H
#include "player_actions.h"

#include <QObject>

class PlayerDialogs : public QObject
{

    Q_OBJECT

public:
    explicit PlayerDialogs(PlayerGraphicsItem *player, QWidget *dialogParent, PlayerActions *playerActions);

signals:
    void requestDialogSemaphore(bool active);

public slots:
    void onViewTopCardsDialogRequested(int defaultNumberTopCards, int deckSize);
    void onViewBottomCardsDialogRequested(int defaultNumberBottomCards, int deckSize);
    void onShuffleTopDialogRequested(int defaultNumberTopCards, int maxCards);
    void onShuffleBottomDialogRequested(int defaultNumberBottomCards, int maxCards);
    void onMulliganDialogRequested(int startSize, int handSize, int deckSize);
    void onDrawCardsDialogRequested(int defaultNumberTopCards, int deckSize);
    void onMoveTopCardsToDialogRequested(int defaultNumberTopCards,
                                         int maxCards,
                                         const QString &targetZone,
                                         const QString &zoneDisplayName,
                                         bool faceDown);
    void onMoveTopCardsUntilDialogRequested(MoveTopCardsUntilOptions options);
    void onMoveBottomCardsToDialogRequested(int defaultNumberBottomCards,
                                            int maxCards,
                                            const QString &targetZone,
                                            const QString &zoneDisplayName,
                                            bool faceDown);
    void onDrawBottomCardsDialogRequested(int defaultNumberBottomCards, int maxCards);
    void onRollDieDialogRequested();
    void onCreateRelatedFromRelationDialogRequested(const CardItem *sourceCard, const CardRelation *cardRelation);
    void onCreateTokenDialogRequested(const QStringList &predefinedTokens);
    void onMoveCardXCardsFromTopDialogRequested(int defaultNumberTopCardsToPlaceBelow, int deckSize);
    void onSetPTDialogRequested(const QString &oldPT);
    void onSetAnnotationDialogRequested(const QString &oldAnnotation);
    void onSetCardCounterDialogRequested(int counterId, const QString &oldValueForDlg);

private:
    PlayerGraphicsItem *player;
    QWidget *dialogParent;
    PlayerActions *playerActions;
};

#endif // COCKATRICE_PLAYER_DIALOGS_H
