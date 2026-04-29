import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';

import { useHandContextMenu } from './useHandContextMenu';

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
  const { handleChoose, handleSameSize, handleMinusOne, handleRevealHand, handleRevealRandom } =
    useHandContextMenu({
      gameId,
      handSize,
      onClose,
      onRequestChooseMulligan,
      onRequestRevealHand,
      onRequestRevealRandom,
    });

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
