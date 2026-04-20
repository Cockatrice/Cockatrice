import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';

import { useWebClient } from '@app/hooks';

import './HandContextMenu.css';

export interface HandContextMenuProps {
  isOpen: boolean;
  anchorPosition: { top: number; left: number } | null;
  gameId: number;
  handSize: number;
  onClose: () => void;
  onRequestChooseMulligan: () => void;
  onRequestRevealHand: () => void;
  onRequestRevealRandom: () => void;
}

function HandContextMenu({
  isOpen,
  anchorPosition,
  gameId,
  handSize,
  onClose,
  onRequestChooseMulligan,
  onRequestRevealHand,
  onRequestRevealRandom,
}: HandContextMenuProps) {
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

  return (
    <Menu
      open={isOpen}
      onClose={onClose}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition ?? undefined}
      data-testid="hand-context-menu"
      className="hand-context-menu"
    >
      <MenuItem onClick={handleChoose}>Take mulligan (choose size)…</MenuItem>
      <MenuItem onClick={handleSameSize} disabled={handSize === 0}>
        Take mulligan (same size)
      </MenuItem>
      <MenuItem onClick={handleMinusOne}>
        Take mulligan (size − 1)
      </MenuItem>
      <Divider />
      <MenuItem onClick={handleRevealHand}>Reveal hand to…</MenuItem>
      <MenuItem onClick={handleRevealRandom} disabled={handSize === 0}>
        Reveal random card to…
      </MenuItem>
    </Menu>
  );
}

export default HandContextMenu;
