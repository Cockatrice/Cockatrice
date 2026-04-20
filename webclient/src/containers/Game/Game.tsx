import { DndContext, DragOverlay } from '@dnd-kit/core';

import {
  AuthGuard,
  CardContextMenu,
  CardDragOverlay,
  CardRegistryContext,
  GameArrowOverlay,
  HandContextMenu,
  HandZone,
  OpponentSelector,
  PhaseBar,
  PlayerBoard,
  PlayerContextMenu,
  RightPanel,
  StackStrip,
  ZoneContextMenu,
} from '@app/components';
import {
  ConfirmDialog,
  CreateCounterDialog,
  CreateTokenDialog,
  DeckSelectDialog,
  GameInfoDialog,
  PromptDialog,
  RevealCardsDialog,
  RollDieDialog,
  SideboardDialog,
  cardsFromZone,
  ZoneViewDialog,
} from '@app/dialogs';
import { App } from '@app/types';

import Layout from '../Layout/Layout';

import { useGame } from './useGame';

import './Game.css';

function Game() {
  const g = useGame();
  const {
    gameId,
    game,
    localPlayer,
    boardRef,
    cardRegistry,
    sensors,
    hoveredCard,
    setHoveredCard,
    isRotated,
    toggleRotated,
    localAccess,
    opponentAccess,
    deckSelectOpen,
    opponents,
    arrows,
    dialogs,
    dnd,
  } = g;

  return (
    <Layout>
      <AuthGuard />
      <CardRegistryContext.Provider value={cardRegistry}>
        <DndContext
          sensors={sensors}
          onDragStart={dnd.handleDragStart}
          onDragEnd={dnd.handleDragEnd}
          onDragCancel={dnd.handleDragCancel}
        >
          <div className="game" data-testid="game-container">
            <PhaseBar gameId={gameId} />

            <div
              className="game__board"
              ref={boardRef}
              onMouseDown={arrows.handleBoardMouseDown}
            >
              <OpponentSelector
                opponents={opponents.opponents}
                selectedPlayerId={opponents.shownOpponentId}
                onSelect={opponents.setSelectedOpponentId}
              />

              {!game && (
                <div className="game__empty" data-testid="game-empty">
                  No active game. Join a game from a room to see the board.
                </div>
              )}

              {game && opponents.shownOpponentId != null && (
                <div
                  className={
                    'game__board-inner' +
                    (isRotated ? ' game__board-inner--rotated' : '')
                  }
                >
                  <PlayerBoard
                    gameId={gameId!}
                    playerId={opponents.shownOpponentId}
                    mirrored
                    canAct={opponentAccess.canAct}
                    canEditCounters={opponentAccess.canAct}
                    arrowSourceKey={arrows.arrowSourceKey}
                    onCardHover={setHoveredCard}
                    onCardClick={arrows.handleCardClick}
                    onCardContextMenu={(card, e) =>
                      dialogs.handleCardContextMenu(opponents.shownOpponentId!, App.ZoneName.TABLE, card, e)
                    }
                    onCardDoubleClick={(card) =>
                      arrows.handleCardDoubleClick(App.ZoneName.TABLE, card)
                    }
                    onZoneClick={dialogs.handleZoneClick}
                    onZoneContextMenu={dialogs.handleZoneContextMenu}
                  />
                  <StackStrip
                    gameId={gameId!}
                    entries={[
                      {
                        playerId: opponents.shownOpponentId,
                        name:
                          opponents.opponents.find((o) => o.playerId === opponents.shownOpponentId)?.name ??
                          `p${opponents.shownOpponentId}`,
                      },
                      {
                        playerId: game.localPlayerId,
                        name:
                          localPlayer?.properties.userInfo?.name ??
                          `p${game.localPlayerId}`,
                      },
                    ]}
                    onZoneClick={dialogs.handleZoneClick}
                  />
                  <PlayerBoard
                    gameId={gameId!}
                    playerId={game.localPlayerId}
                    canAct={localAccess.canAct}
                    canEditCounters={localAccess.canAct}
                    arrowSourceKey={arrows.arrowSourceKey}
                    onCardHover={setHoveredCard}
                    onCardClick={arrows.handleCardClick}
                    onCardContextMenu={(card, e) =>
                      dialogs.handleCardContextMenu(game.localPlayerId, App.ZoneName.TABLE, card, e)
                    }
                    onCardDoubleClick={(card) =>
                      arrows.handleCardDoubleClick(App.ZoneName.TABLE, card)
                    }
                    onZoneClick={dialogs.handleZoneClick}
                    onZoneContextMenu={dialogs.handleZoneContextMenu}
                    onRequestCreateCounter={dialogs.openCreateCounter}
                    onPlayerContextMenu={dialogs.handlePlayerContextMenu}
                  />
                  {localPlayer && (
                    <HandZone
                      gameId={gameId!}
                      playerId={game.localPlayerId}
                      canAct={localAccess.canAct}
                      arrowSourceKey={arrows.arrowSourceKey}
                      onCardHover={setHoveredCard}
                      onCardClick={arrows.handleCardClick}
                      onCardContextMenu={(card, e) =>
                        dialogs.handleCardContextMenu(game.localPlayerId, App.ZoneName.HAND, card, e)
                      }
                      onZoneContextMenu={dialogs.handleHandContextMenu}
                    />
                  )}
                </div>
              )}

              <GameArrowOverlay gameId={gameId} boardRef={boardRef} dragPreview={arrows.dragPreview} />
            </div>

            <RightPanel
              gameId={gameId}
              hoveredCard={hoveredCard}
              onRequestRollDie={dialogs.openRollDie}
              onRequestConcede={dialogs.openConcede}
              onRequestUnconcede={dialogs.openUnconcede}
              onRequestGameInfo={dialogs.openGameInfo}
              onToggleRotate90={toggleRotated}
              isRotated={isRotated}
            />

            <DeckSelectDialog isOpen={deckSelectOpen} gameId={gameId} />

            {dialogs.zoneViews.map((v, idx) => (
              <ZoneViewDialog
                key={`${v.playerId}-${v.zoneName}`}
                isOpen
                gameId={gameId}
                playerId={v.playerId}
                zoneName={v.zoneName}
                handleClose={() => dialogs.handleCloseZoneView(v.playerId, v.zoneName)}
                initialPosition={{ x: 80 + idx * 36, y: 80 + idx * 36 }}
              />
            ))}

            <CardContextMenu
              isOpen={dialogs.cardMenu != null}
              anchorPosition={dialogs.cardMenu?.anchorPosition ?? null}
              gameId={gameId ?? 0}
              localPlayerId={game?.localPlayerId ?? null}
              card={dialogs.cardMenu?.card ?? null}
              ownerPlayerId={dialogs.cardMenu?.sourcePlayerId ?? null}
              sourceZone={dialogs.cardMenu?.sourceZone ?? null}
              onClose={dialogs.closeCardMenu}
              onRequestSetPT={dialogs.handleRequestSetPT}
              onRequestSetAnnotation={dialogs.handleRequestSetAnnotation}
              onRequestSetCounter={dialogs.handleRequestSetCardCounter}
              onRequestDrawArrow={dialogs.handleRequestDrawArrow}
              onRequestAttach={dialogs.handleRequestAttach}
              onRequestMoveToLibraryAt={dialogs.handleRequestMoveToLibraryAt}
            />

            <ZoneContextMenu
              isOpen={dialogs.zoneMenu != null}
              anchorPosition={dialogs.zoneMenu?.anchorPosition ?? null}
              gameId={gameId ?? 0}
              playerId={dialogs.zoneMenu?.playerId ?? null}
              zoneName={dialogs.zoneMenu?.zoneName ?? null}
              onClose={dialogs.closeZoneMenu}
              onRequestDrawN={dialogs.handleRequestDrawN}
              onRequestDumpN={dialogs.handleRequestDumpN}
              onRequestRevealTopN={dialogs.handleRequestRevealTopN}
              onRequestRevealZone={dialogs.handleRequestRevealZone}
            />

            <PlayerContextMenu
              isOpen={dialogs.playerMenu != null}
              anchorPosition={dialogs.playerMenu}
              onClose={dialogs.closePlayerMenu}
              onRequestCreateToken={dialogs.openCreateToken}
              onRequestViewSideboard={dialogs.openSideboard}
            />

            <HandContextMenu
              isOpen={dialogs.handMenu != null}
              anchorPosition={dialogs.handMenu}
              gameId={gameId ?? 0}
              handSize={localPlayer?.zones[App.ZoneName.HAND]?.cardCount ?? 0}
              onClose={dialogs.closeHandMenu}
              onRequestChooseMulligan={dialogs.handleRequestChooseMulligan}
              onRequestRevealHand={dialogs.handleRequestRevealHand}
              onRequestRevealRandom={dialogs.handleRequestRevealRandom}
            />

            {dialogs.prompt && (
              <PromptDialog
                isOpen
                title={dialogs.prompt.title}
                label={dialogs.prompt.label}
                initialValue={dialogs.prompt.initialValue}
                helperText={dialogs.prompt.helperText}
                validate={dialogs.prompt.validate}
                onSubmit={dialogs.prompt.onSubmit}
                onCancel={dialogs.closePrompt}
              />
            )}

            <RollDieDialog
              isOpen={dialogs.rollDieOpen}
              lastSides={dialogs.lastDieSides}
              lastCount={dialogs.lastDieCount}
              onSubmit={dialogs.handleRollDieSubmit}
              onCancel={dialogs.closeRollDie}
            />

            <CreateCounterDialog
              isOpen={dialogs.createCounterOpen}
              onSubmit={dialogs.handleCreateCounterSubmit}
              onCancel={dialogs.closeCreateCounter}
            />

            <CreateTokenDialog
              isOpen={dialogs.createTokenOpen}
              onSubmit={dialogs.handleCreateTokenSubmit}
              onCancel={dialogs.closeCreateToken}
            />

            <SideboardDialog
              isOpen={dialogs.sideboardOpen}
              playerName={localPlayer?.properties.userInfo?.name ?? ''}
              deckCards={cardsFromZone(localPlayer?.zones[App.ZoneName.DECK])}
              sideboardCards={cardsFromZone(localPlayer?.zones[App.ZoneName.SIDEBOARD])}
              isLocked={localPlayer?.properties.sideboardLocked ?? false}
              onSubmit={dialogs.handleSideboardSubmit}
              onCancel={dialogs.closeSideboard}
              onToggleLock={dialogs.handleToggleSideboardLock}
            />

            {dialogs.revealState && (
              <RevealCardsDialog
                isOpen
                title={dialogs.revealState.title}
                zoneLabel={dialogs.revealState.zoneLabel}
                showCountInput={dialogs.revealState.showCountInput}
                defaultCount={dialogs.revealState.defaultCount}
                players={opponents.revealPlayers}
                onSubmit={dialogs.revealState.onSubmit}
                onCancel={dialogs.closeReveal}
              />
            )}

            <ConfirmDialog
              isOpen={dialogs.concedeConfirm === 'concede'}
              title="Concede this game?"
              message="You'll stay seated as a spectator until you click Unconcede or Leave Game. Others will see you as conceded."
              confirmLabel="Concede"
              destructive
              onConfirm={dialogs.confirmConcede}
              onCancel={dialogs.closeConcedeConfirm}
            />

            <ConfirmDialog
              isOpen={dialogs.concedeConfirm === 'unconcede'}
              title="Rejoin the game?"
              message="This undoes your concede and puts you back into the active player rotation."
              confirmLabel="Unconcede"
              onConfirm={dialogs.confirmUnconcede}
              onCancel={dialogs.closeConcedeConfirm}
            />

            <GameInfoDialog
              isOpen={dialogs.gameInfoOpen}
              gameId={gameId}
              onClose={dialogs.closeGameInfo}
            />
          </div>

          <DragOverlay>
            {dnd.activeCard ? <CardDragOverlay card={dnd.activeCard} /> : null}
          </DragOverlay>
        </DndContext>
      </CardRegistryContext.Provider>
    </Layout>
  );
}

export default Game;
