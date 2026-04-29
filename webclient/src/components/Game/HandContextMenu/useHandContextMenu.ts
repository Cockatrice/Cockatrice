import { useWebClient } from '@app/hooks';

export interface HandContextMenu {
  handleChoose: () => void;
  handleSameSize: () => void;
  handleMinusOne: () => void;
  handleRevealHand: () => void;
  handleRevealRandom: () => void;
}

export interface UseHandContextMenuArgs {
  gameId: number;
  handSize: number;
  onClose: () => void;
  onRequestChooseMulligan: () => void;
  onRequestRevealHand: () => void;
  onRequestRevealRandom: () => void;
}

export function useHandContextMenu({
  gameId,
  handSize,
  onClose,
  onRequestChooseMulligan,
  onRequestRevealHand,
  onRequestRevealRandom,
}: UseHandContextMenuArgs): HandContextMenu {
  const webClient = useWebClient();

  const handleChoose = () => {
    if (gameId <= 0) {
      return;
    }
    onRequestChooseMulligan();
    onClose();
  };

  const handleSameSize = () => {
    if (gameId <= 0) {
      return;
    }
    webClient.request.game.mulligan(gameId, { number: handSize });
    onClose();
  };

  const handleMinusOne = () => {
    if (gameId <= 0) {
      return;
    }
    // Desktop's actMulliganMinusOne floors at 1 (see
    // cockatrice/src/game/player/player_actions.cpp actMulliganMinusOne);
    // the server-side doMulligan rejects number < 1.
    const next = Math.max(1, handSize - 1);
    webClient.request.game.mulligan(gameId, { number: next });
    onClose();
  };

  const handleRevealHand = () => {
    if (gameId <= 0) {
      return;
    }
    onRequestRevealHand();
    onClose();
  };

  const handleRevealRandom = () => {
    if (gameId <= 0) {
      return;
    }
    onRequestRevealRandom();
    onClose();
  };

  return { handleChoose, handleSameSize, handleMinusOne, handleRevealHand, handleRevealRandom };
}
