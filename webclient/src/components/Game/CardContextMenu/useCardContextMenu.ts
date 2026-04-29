import { useWebClient } from '@app/hooks';
import { App, Data } from '@app/types';

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
export const CARD_MOVE_TARGETS: ReadonlyArray<MoveTarget> = [
  { label: 'Send to Hand', zone: App.ZoneName.HAND, x: -1, y: 0 },
  { label: 'Send to Battlefield', zone: App.ZoneName.TABLE, x: 0, y: 0 },
  { label: 'Send to Graveyard', zone: App.ZoneName.GRAVE, x: 0, y: 0 },
  { label: 'Send to Exile', zone: App.ZoneName.EXILE, x: 0, y: 0 },
  { label: 'Send to Library (top)', zone: App.ZoneName.DECK, x: 0, y: 0 },
  { label: 'Send to Library (bottom)', zone: App.ZoneName.DECK, x: -1, y: 0 },
];

export interface CardContextMenu {
  ready: boolean;
  isOwnedByLocal: boolean;
  canAttach: boolean;
  isAttached: boolean;
  moveTargets: ReadonlyArray<MoveTarget>;
  handleFlip: () => void;
  handleTapToggle: () => void;
  handleFaceDownToggle: () => void;
  handleDoesntUntapToggle: () => void;
  handleSetPT: () => void;
  handleSetAnnotation: () => void;
  handleCardCounterDelta: (delta: number) => void;
  handleSetCardCounter: () => void;
  handleDrawArrow: () => void;
  handleAttach: () => void;
  handleUnattach: () => void;
  handleMove: (target: MoveTarget) => void;
  handleMoveToLibraryAt: () => void;
}

export interface UseCardContextMenuArgs {
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

export function useCardContextMenu({
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
}: UseCardContextMenuArgs): CardContextMenu {
  const webClient = useWebClient();

  const ready = card != null && ownerPlayerId != null && sourceZone != null && localPlayerId != null;

  // Mutating actions (tap, flip, counters, attrs, P/T, annotation, attach,
  // move) require ownership of the card — matches desktop's
  // `card_menu.cpp:151-161` which drops all mutators when the menu target
  // isn't getLocalOrJudge()-modifiable. Read-only actions (Draw arrow)
  // stay available for planning/communication.
  const isOwnedByLocal = ready && ownerPlayerId === localPlayerId;
  const isAttached = ready && (card!.attachCardId ?? -1) >= 0;
  // Desktop's actAttach is only available from a table card; other zones
  // never expose the attach arrow.
  const canAttach = ready && sourceZone === App.ZoneName.TABLE;

  const setAttr = (attribute: Data.CardAttribute, value: string) => {
    if (!ready) {
      return;
    }
    webClient.request.game.setCardAttr(gameId, {
      zone: sourceZone!,
      cardId: card!.id,
      attribute,
      attrValue: value,
    });
  };

  const handleFlip = () => {
    if (!ready) {
      return;
    }
    // TODO(card-db): desktop's Player::actCardMenuFlip reads the card's stored
    // P/T and forwards it so the revealed side shows the correct stats
    // (cockatrice/src/game/player/player_actions.cpp:1805-1810). We can't
    // do that without a card-database-by-name lookup, which isn't wired in
    // the webclient yet. The server re-derives PT from the card DB for known
    // names, so omitting `pt` is harmless for non-custom cards.
    webClient.request.game.flipCard(gameId, {
      zone: sourceZone!,
      cardId: card!.id,
      faceDown: !card!.faceDown,
    });
    onClose();
  };

  const handleTapToggle = () => {
    if (!ready) {
      return;
    }
    setAttr(Data.CardAttribute.AttrTapped, card!.tapped ? '0' : '1');
    onClose();
  };

  const handleFaceDownToggle = () => {
    if (!ready) {
      return;
    }
    setAttr(Data.CardAttribute.AttrFaceDown, card!.faceDown ? '0' : '1');
    onClose();
  };

  const handleDoesntUntapToggle = () => {
    if (!ready) {
      return;
    }
    setAttr(Data.CardAttribute.AttrDoesntUntap, card!.doesntUntap ? '0' : '1');
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
    if (!ready) {
      return;
    }
    webClient.request.game.incCardCounter(gameId, {
      zone: sourceZone!,
      cardId: card!.id,
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
    if (!ready) {
      return;
    }
    // Desktop's actUnattach sends only start_zone + card_id; the server uses
    // proto2 presence (`has_target_player_id()`) to detect "detach". Setting
    // targetPlayerId: -1 here would leave presence set and trip the attach
    // code path server-side. MessageInitShape makes these fields optional,
    // so omitting them produces an unset wire field.
    webClient.request.game.attachCard(gameId, { startZone: sourceZone!, cardId: card!.id });
    onClose();
  };

  const handleMove = (target: MoveTarget) => {
    if (!ready) {
      return;
    }
    // targetPlayerId is the ACTING player (local), matching desktop's
    // Player::actMoveCardTo* which uses playerInfo->getId().
    webClient.request.game.moveCard(gameId, {
      startPlayerId: ownerPlayerId!,
      startZone: sourceZone!,
      cardsToMove: { card: [{ cardId: card!.id }] },
      targetPlayerId: localPlayerId!,
      targetZone: target.zone,
      x: target.x,
      y: target.y,
      isReversed: false,
    });
    onClose();
  };

  const handleMoveToLibraryAt = () => {
    onRequestMoveToLibraryAt();
    onClose();
  };

  return {
    ready,
    isOwnedByLocal,
    canAttach,
    isAttached,
    moveTargets: CARD_MOVE_TARGETS,
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
  };
}
