import { useCurrentGame, useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { App, Data } from '@app/types';

export interface PhaseBar {
  activePhase: App.Phase | undefined;
  canAdvance: boolean;
  handlePhaseClick: (phase: App.Phase) => void;
  handlePass: () => void;
  handleUntapAll: () => void;
  handleDrawOne: () => void;
}

export function usePhaseBar(gameId: number | undefined): PhaseBar {
  const webClient = useWebClient();
  const { game, isJudge, isStarted } = useCurrentGame(gameId);
  const activePhase = useAppSelector((state) =>
    gameId != null ? GameSelectors.getActivePhase(state, gameId) : undefined,
  );
  const localPlayerId = game?.localPlayerId;
  const tableCards = useAppSelector((state) =>
    gameId != null && localPlayerId != null
      ? GameSelectors.getCards(state, gameId, localPlayerId, App.ZoneName.TABLE)
      : undefined,
  );

  // Desktop: only the active player (or a judge) can advance the phase.
  const canAdvance =
    gameId != null &&
    game != null &&
    isStarted &&
    (isJudge || game.activePlayerId === game.localPlayerId);

  const handlePhaseClick = (phase: App.Phase) => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.setActivePhase(gameId, { phase });
  };

  const handlePass = () => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.nextTurn(gameId);
  };

  // Desktop's untap-step double-click fires "Untap All" on the local player's
  // table zone (cockatrice/src/game/player/player_actions.cpp actUntapAll).
  // We replicate by sending one setCardAttr per tapped card; there is no
  // batch variant on the wire.
  const handleUntapAll = () => {
    if (!canAdvance || gameId == null || !tableCards) {
      return;
    }
    for (const card of tableCards) {
      if (card.tapped) {
        webClient.request.game.setCardAttr(gameId, {
          zone: App.ZoneName.TABLE,
          cardId: card.id,
          attribute: Data.CardAttribute.AttrTapped,
          attrValue: '0',
        });
      }
    }
  };

  const handleDrawOne = () => {
    if (!canAdvance || gameId == null) {
      return;
    }
    webClient.request.game.drawCards(gameId, { number: 1 });
  };

  return { activePhase, canAdvance, handlePhaseClick, handlePass, handleUntapAll, handleDrawOne };
}
