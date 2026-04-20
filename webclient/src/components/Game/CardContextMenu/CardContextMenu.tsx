import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import Divider from '@mui/material/Divider';

import { useWebClient } from '@app/hooks';
import { App, Data } from '@app/types';

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

interface MoveTarget {
  label: string;
  zone: string;
  x: number;
  y: number;
}

// Mirrors desktop's cockatrice/src/game/player/menu/move_menu.cpp:32-42 —
// six fixed targets plus one prompt ("Move to library at position…") for the
// 7-entry parity. Note that desktop's "Send to Table" label maps to our
// "Send to Battlefield" (same wire semantics: zone=table, x=0, y=0); the
// label diverges but the command is identical.
const MOVE_TARGETS: ReadonlyArray<MoveTarget> = [
  { label: 'Send to Hand', zone: App.ZoneName.HAND, x: -1, y: 0 },
  { label: 'Send to Battlefield', zone: App.ZoneName.TABLE, x: 0, y: 0 },
  { label: 'Send to Graveyard', zone: App.ZoneName.GRAVE, x: 0, y: 0 },
  { label: 'Send to Exile', zone: App.ZoneName.EXILE, x: 0, y: 0 },
  { label: 'Send to Library (top)', zone: App.ZoneName.DECK, x: 0, y: 0 },
  { label: 'Send to Library (bottom)', zone: App.ZoneName.DECK, x: -1, y: 0 },
];

function CardContextMenu({
  isOpen,
  anchorPosition,
  gameId,
  localPlayerId,
  card,
  ownerPlayerId,
  sourceZone,
  onClose,
  onRequestSetPT,
  onRequestSetAnnotation,
  onRequestSetCounter,
  onRequestDrawArrow,
  onRequestAttach,
  onRequestMoveToLibraryAt,
}: CardContextMenuProps) {
  const webClient = useWebClient();

  if (!card || ownerPlayerId == null || sourceZone == null || localPlayerId == null) {
    return null;
  }

  const game = webClient.request.game;
  const zone = sourceZone;
  const cardId = card.id;

  const setAttr = (attribute: Data.CardAttribute, value: string) => {
    game.setCardAttr(gameId, { zone, cardId, attribute, attrValue: value });
  };

  const handleFlip = () => {
    // TODO(card-db): desktop's Player::actCardMenuFlip reads the card's stored
    // P/T and forwards it so the revealed side shows the correct stats
    // (cockatrice/src/game/player/player_actions.cpp:1805-1810). We can't
    // do that without a card-database-by-name lookup, which isn't wired in
    // the webclient yet. The server re-derives PT from the card DB for known
    // names, so omitting `pt` is harmless for non-custom cards.
    game.flipCard(gameId, { zone, cardId, faceDown: !card.faceDown });
    onClose();
  };

  const handleTapToggle = () => {
    setAttr(Data.CardAttribute.AttrTapped, card.tapped ? '0' : '1');
    onClose();
  };

  const handleFaceDownToggle = () => {
    setAttr(Data.CardAttribute.AttrFaceDown, card.faceDown ? '0' : '1');
    onClose();
  };

  const handleDoesntUntapToggle = () => {
    setAttr(Data.CardAttribute.AttrDoesntUntap, card.doesntUntap ? '0' : '1');
    onClose();
  };

  const handleSetPT = () => {
    onRequestSetPT();
    onClose();
  };

  const handleSetAnnotation = () => {
    onRequestSetAnnotation();
    onClose();
  };

  const handleCardCounterDelta = (delta: number) => {
    game.incCardCounter(gameId, {
      zone,
      cardId,
      counterId: 0,
      counterDelta: delta,
    });
    onClose();
  };

  const handleSetCardCounter = () => {
    onRequestSetCounter();
    onClose();
  };

  const handleDrawArrow = () => {
    onRequestDrawArrow();
    onClose();
  };

  const handleAttach = () => {
    onRequestAttach();
    onClose();
  };

  const handleUnattach = () => {
    // Desktop's actUnattach sends only start_zone + card_id; the server uses
    // proto2 presence (`has_target_player_id()`) to detect "detach". Setting
    // targetPlayerId: -1 here would leave presence set and trip the attach
    // code path server-side. MessageInitShape makes these fields optional,
    // so omitting them produces an unset wire field.
    game.attachCard(gameId, { startZone: zone, cardId });
    onClose();
  };

  const isAttached = card.attachCardId >= 0;
  // Desktop's actAttach is only available from a table card; other zones
  // never expose the attach arrow.
  const canAttach = sourceZone === App.ZoneName.TABLE;

  // Mutating actions (tap, flip, counters, attrs, P/T, annotation, attach,
  // move) require ownership of the card — matches desktop's
  // `card_menu.cpp:151-161` which drops all mutators when the menu target
  // isn't getLocalOrJudge()-modifiable. Read-only actions (Draw arrow)
  // stay available for planning/communication.
  const isOwnedByLocal = ownerPlayerId === localPlayerId;

  const handleMove = (target: MoveTarget) => {
    // targetPlayerId is the ACTING player (local), matching desktop's
    // Player::actMoveCardTo* which uses playerInfo->getId().
    game.moveCard(gameId, {
      startPlayerId: ownerPlayerId,
      startZone: sourceZone,
      cardsToMove: { card: [{ cardId }] },
      targetPlayerId: localPlayerId,
      targetZone: target.zone,
      x: target.x,
      y: target.y,
      isReversed: false,
    });
    onClose();
  };

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
          {MOVE_TARGETS.map((t) => (
            <MenuItem key={t.label} onClick={() => handleMove(t)}>
              {t.label}
            </MenuItem>
          ))}
          <MenuItem
            onClick={() => {
              onRequestMoveToLibraryAt();
              onClose();
            }}
          >
            Move to library at position…
          </MenuItem>
        </>
      )}
    </Menu>
  );
}

export default CardContextMenu;
