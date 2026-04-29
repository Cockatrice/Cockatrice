import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';

import { Data } from '@app/types';

import { useCardContextMenu } from './useCardContextMenu';

import './CardContextMenu.css';

export interface CardContextMenuProps {
  isOpen: boolean;
  anchorPosition: { top: number; left: number } | null;
  gameId: number;
  localPlayerId: number | null;
  card: Data.ServerInfo_Card | null;
  ownerPlayerId: number | null;
  sourceZone: string | null;
  onClose: () => void;
  onRequestSetPT: () => void;
  onRequestSetAnnotation: () => void;
  onRequestSetCounter: () => void;
  onRequestDrawArrow: () => void;
  onRequestAttach: () => void;
  onRequestMoveToLibraryAt: () => void;
}

function CardContextMenu(props: CardContextMenuProps) {
  const { isOpen, anchorPosition, card, onClose } = props;
  const {
    ready,
    isOwnedByLocal,
    canAttach,
    isAttached,
    moveTargets,
    handleFlip,
    handleTapToggle,
    handleFaceDownToggle,
    handleDoesntUntapToggle,
    handleSetPT,
    handleSetAnnotation,
    handleCardCounterDelta,
    handleSetCardCounter,
    handleDrawArrow,
    handleAttach,
    handleUnattach,
    handleMove,
    handleMoveToLibraryAt,
  } = useCardContextMenu(props);

  if (!ready || !card) {
    return null;
  }

  return (
    <Menu
      open={isOpen}
      onClose={onClose}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition ?? undefined}
      data-testid="card-context-menu"
      className="card-context-menu"
    >
      {isOwnedByLocal && (
        <>
          <MenuItem onClick={handleFlip}>Flip</MenuItem>
          <MenuItem onClick={handleTapToggle}>{card.tapped ? 'Untap' : 'Tap'}</MenuItem>
          <MenuItem onClick={handleFaceDownToggle}>
            {card.faceDown ? 'Face Up' : 'Face Down'}
          </MenuItem>
          <MenuItem onClick={handleDoesntUntapToggle}>
            {card.doesntUntap ? 'Allow Untap' : 'Doesn\'t Untap'}
          </MenuItem>
          <MenuItem onClick={handleSetPT}>Set P/T…</MenuItem>
          <MenuItem onClick={handleSetAnnotation}>Set Annotation…</MenuItem>
          <Divider />
          <MenuItem onClick={() => handleCardCounterDelta(+1)}>Add counter</MenuItem>
          <MenuItem onClick={() => handleCardCounterDelta(-1)}>Remove counter</MenuItem>
          <MenuItem onClick={handleSetCardCounter}>Set counter…</MenuItem>
          <Divider />
        </>
      )}
      <MenuItem onClick={handleDrawArrow}>Draw arrow from here</MenuItem>
      {isOwnedByLocal && canAttach && (
        <MenuItem onClick={handleAttach}>Attach to card…</MenuItem>
      )}
      {isOwnedByLocal && canAttach && isAttached && (
        <MenuItem onClick={handleUnattach}>Unattach</MenuItem>
      )}
      {isOwnedByLocal && (
        <>
          <Divider />
          {moveTargets.map((t) => (
            <MenuItem key={t.label} onClick={() => handleMove(t)}>
              {t.label}
            </MenuItem>
          ))}
          <MenuItem onClick={handleMoveToLibraryAt}>
            Move to library at position…
          </MenuItem>
        </>
      )}
    </Menu>
  );
}

export default CardContextMenu;
