import { useEffect, useMemo, useRef, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import {
  DndContext,
  DragEndEvent,
  DragOverlay,
  DragStartEvent,
  KeyboardSensor,
  PointerSensor,
  useSensor,
  useSensors,
} from '@dnd-kit/core';

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
  createCardRegistry,
  makeCardKey,
  useToast,
} from '@app/components';
import {
  ConfirmDialog,
  CreateCounterDialog,
  CreateTokenDialog,
  DeckSelectDialog,
  DEFAULT_DIE_COUNT,
  DEFAULT_DIE_SIDES,
  GameInfoDialog,
  PromptDialog,
  RevealCardsDialog,
  RollDieDialog,
  SideboardDialog,
  cardsFromZone,
  type SideboardPlanMove,
  ZoneViewDialog,
} from '@app/dialogs';
import {
  useCurrentGame,
  useGameAccess,
  useGameLifecycle,
  useWebClient,
} from '@app/hooks';
import { App, Data } from '@app/types';

import Layout from '../Layout/Layout';

import './Game.css';

interface ZoneViewTarget {
  playerId: number;
  zoneName: string;
}

interface CardMenuState {
  card: Data.ServerInfo_Card;
  sourcePlayerId: number;
  sourceZone: string;
  anchorPosition: { top: number; left: number };
}

interface ZoneMenuState {
  playerId: number;
  zoneName: string;
  anchorPosition: { top: number; left: number };
}

interface PromptState {
  title: string;
  label: string;
  initialValue?: string;
  helperText?: string;
  validate?: (value: string) => string | null;
  onSubmit: (value: string) => void;
}

interface PendingArrow {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
}

// Shares shape with PendingArrow today, but kept distinct so future
// protocol fields (e.g. desktop's attach-target coord hints) can diverge
// without a runtime switch.
interface PendingAttach {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
}

interface AnchorPosition {
  top: number;
  left: number;
}

interface RevealState {
  title: string;
  zoneName: string;
  zoneLabel: string;
  showCountInput: boolean;
  defaultCount: number;
  onSubmit: (args: { targetPlayerId: number; topCards: number }) => void;
}

interface ArrowDragState {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
  startX: number;
  startY: number;
  currentX: number;
  currentY: number;
  moved: boolean;
}

function arrowColorForModifiers(e: { ctrlKey: boolean; altKey: boolean; shiftKey: boolean }): App.ColorRGBA {
  if (e.ctrlKey) {
    return App.ArrowColor.YELLOW;
  }
  if (e.altKey) {
    return App.ArrowColor.BLUE;
  }
  if (e.shiftKey) {
    return App.ArrowColor.GREEN;
  }
  return App.ArrowColor.RED;
}

const ARROW_DRAG_THRESHOLD_PX = 8;

function Game() {
  const { gameId, game, localPlayer, isSpectator, isJudge } = useCurrentGame();
  const webClient = useWebClient();
  const navigate = useNavigate();

  const kickedToast = useToast({
    key: 'game-kicked',
    children: 'You were kicked from the game',
  });
  const gameClosedToast = useToast({
    key: 'game-closed',
    children: 'The game was closed by the host',
  });

  useGameLifecycle(gameId, {
    onKicked: () => {
      kickedToast.openToast();
      navigate(App.RouteEnum.SERVER);
    },
    onGameClosed: () => {
      gameClosedToast.openToast();
      navigate(App.RouteEnum.SERVER);
    },
  });

  const [hoveredCard, setHoveredCard] = useState<Data.ServerInfo_Card | null>(null);
  const [selectedOpponentId, setSelectedOpponentId] = useState<number | undefined>();
  const [zoneViews, setZoneViews] = useState<ZoneViewTarget[]>([]);
  const [activeCard, setActiveCard] = useState<Data.ServerInfo_Card | null>(null);
  const [cardMenu, setCardMenu] = useState<CardMenuState | null>(null);
  const [zoneMenu, setZoneMenu] = useState<ZoneMenuState | null>(null);
  const [prompt, setPrompt] = useState<PromptState | null>(null);
  const [rollDieOpen, setRollDieOpen] = useState(false);
  const [lastDieSides, setLastDieSides] = useState(DEFAULT_DIE_SIDES);
  const [lastDieCount, setLastDieCount] = useState(DEFAULT_DIE_COUNT);
  const [createCounterOpen, setCreateCounterOpen] = useState(false);
  const [createTokenOpen, setCreateTokenOpen] = useState(false);
  const [sideboardOpen, setSideboardOpen] = useState(false);
  const [revealState, setRevealState] = useState<RevealState | null>(null);
  const [playerMenu, setPlayerMenu] = useState<AnchorPosition | null>(null);
  const [handMenu, setHandMenu] = useState<AnchorPosition | null>(null);
  const [pendingArrow, setPendingArrow] = useState<PendingArrow | null>(null);
  const [pendingAttach, setPendingAttach] = useState<PendingAttach | null>(null);
  const [arrowDrag, setArrowDrag] = useState<ArrowDragState | null>(null);
  const [concedeConfirm, setConcedeConfirm] = useState<'concede' | 'unconcede' | null>(null);
  const [gameInfoOpen, setGameInfoOpen] = useState(false);
  // View-only 90° rotation; local to this tab, mirrors desktop's
  // Player::actRotateLocal which applies a QGraphicsView transform with no
  // server call.
  const [isRotated, setIsRotated] = useState(false);

  const boardRef = useRef<HTMLDivElement>(null);
  const suppressNextContextMenuRef = useRef(false);
  const cardRegistry = useMemo(() => createCardRegistry(), []);
  const sensors = useSensors(useSensor(PointerSensor), useSensor(KeyboardSensor));

  const opponents = useMemo(() => {
    if (!game) {
      return [];
    }
    return Object.values(game.players)
      .filter((p) => p.properties.playerId !== game.localPlayerId)
      .map((p) => ({
        playerId: p.properties.playerId,
        name: p.properties.userInfo?.name ?? `p${p.properties.playerId}`,
      }));
  }, [game]);

  useEffect(() => {
    if (selectedOpponentId == null && opponents.length > 0) {
      setSelectedOpponentId(opponents[0].playerId);
    }
    if (selectedOpponentId != null && !opponents.some((o) => o.playerId === selectedOpponentId)) {
      setSelectedOpponentId(opponents[0]?.playerId);
    }
  }, [opponents, selectedOpponentId]);

  // ESC cancels a pending arrow OR attach (matches desktop). Suppress the
  // cancel when a MUI dialog is open — the dialog's own ESC handler should
  // win so the user isn't rug-pulled out of a modal form.
  useEffect(() => {
    if (!pendingArrow && !pendingAttach && !arrowDrag) {
      return undefined;
    }
    const handler = (e: KeyboardEvent) => {
      if (e.key !== 'Escape') {
        return;
      }
      if (document.querySelector('.MuiDialog-root[role="dialog"]')) {
        return;
      }
      setPendingArrow(null);
      setPendingAttach(null);
      setArrowDrag(null);
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [pendingArrow, pendingAttach, arrowDrag]);

  // Right-click-drag arrow-draw lifecycle: window-level mousemove + mouseup
  // listeners that track the cursor and finalize on release.
  useEffect(() => {
    if (!arrowDrag) {
      return undefined;
    }

    const handleMove = (e: MouseEvent) => {
      setArrowDrag((prev) => {
        if (!prev) {
          return prev;
        }
        const movedX = Math.abs(e.clientX - prev.startX);
        const movedY = Math.abs(e.clientY - prev.startY);
        const moved = prev.moved || movedX + movedY > ARROW_DRAG_THRESHOLD_PX;
        return { ...prev, currentX: e.clientX, currentY: e.clientY, moved };
      });
    };

    const handleUp = (e: MouseEvent) => {
      if (e.button !== 2) {
        return;
      }
      const drag = arrowDrag;
      if (!drag) {
        return;
      }
      const movedX = Math.abs(e.clientX - drag.startX);
      const movedY = Math.abs(e.clientY - drag.startY);
      const moved = drag.moved || movedX + movedY > ARROW_DRAG_THRESHOLD_PX;
      setArrowDrag(null);
      if (!moved || gameId == null) {
        // Short right-click with no drag: let the contextmenu handler run
        // (it will open the card menu).
        return;
      }
      // Any real drag suppresses the contextmenu event that follows mouseup.
      suppressNextContextMenuRef.current = true;

      const el = document.elementFromPoint(e.clientX, e.clientY)?.closest('[data-card-id]') as HTMLElement | null;
      if (!el) {
        return;
      }
      const targetPlayerId = Number(el.getAttribute('data-card-owner'));
      const targetZone = el.getAttribute('data-card-zone') ?? '';
      const targetCardId = Number(el.getAttribute('data-card-id'));
      if (!Number.isFinite(targetPlayerId) || !targetZone || !Number.isFinite(targetCardId)) {
        return;
      }
      // Same-card drops are cancellations.
      if (
        targetPlayerId === drag.sourcePlayerId &&
        targetZone === drag.sourceZone &&
        targetCardId === drag.sourceCardId
      ) {
        return;
      }
      // Desktop parity: dragging an arrow from a local-hand card to a target
      // outside the hand auto-plays the card (card_item.cpp:243-250) — the
      // card is moved to the battlefield before any arrow is drawn. The
      // server re-keys the card id during the move, so we can't also send
      // createArrow here; instead we resolve this drag as a play-card intent.
      if (
        drag.sourceZone === App.ZoneName.HAND &&
        drag.sourcePlayerId === game?.localPlayerId &&
        targetZone !== App.ZoneName.HAND
      ) {
        webClient.request.game.moveCard(gameId, {
          startPlayerId: drag.sourcePlayerId,
          startZone: drag.sourceZone,
          cardsToMove: { card: [{ cardId: drag.sourceCardId }] },
          targetPlayerId: drag.sourcePlayerId,
          targetZone: App.ZoneName.TABLE,
          x: 0,
          y: 0,
          isReversed: false,
        });
        return;
      }
      webClient.request.game.createArrow(gameId, {
        startPlayerId: drag.sourcePlayerId,
        startZone: drag.sourceZone,
        startCardId: drag.sourceCardId,
        targetPlayerId,
        targetZone,
        targetCardId,
        arrowColor: arrowColorForModifiers(e),
      });
    };

    window.addEventListener('mousemove', handleMove);
    window.addEventListener('mouseup', handleUp);
    return () => {
      window.removeEventListener('mousemove', handleMove);
      window.removeEventListener('mouseup', handleUp);
    };
  }, [arrowDrag, gameId, webClient]);

  // Suppress the browser contextmenu event after a right-drag.
  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (suppressNextContextMenuRef.current) {
        e.preventDefault();
        suppressNextContextMenuRef.current = false;
      }
    };
    window.addEventListener('contextmenu', handler);
    return () => window.removeEventListener('contextmenu', handler);
  }, []);

  const handleBoardMouseDown = (e: React.MouseEvent<HTMLDivElement>) => {
    if (e.button !== 2) {
      return;
    }
    const el = (e.target as HTMLElement).closest('[data-card-id]') as HTMLElement | null;
    if (!el) {
      return;
    }
    const sourcePlayerId = Number(el.getAttribute('data-card-owner'));
    const sourceZone = el.getAttribute('data-card-zone') ?? '';
    const sourceCardId = Number(el.getAttribute('data-card-id'));
    if (!Number.isFinite(sourcePlayerId) || !sourceZone || !Number.isFinite(sourceCardId)) {
      return;
    }
    setArrowDrag({
      sourcePlayerId,
      sourceZone,
      sourceCardId,
      startX: e.clientX,
      startY: e.clientY,
      currentX: e.clientX,
      currentY: e.clientY,
      moved: false,
    });
  };

  const shownOpponentId = selectedOpponentId ?? opponents[0]?.playerId;

  const localAccess = useGameAccess(gameId, game?.localPlayerId);
  const opponentAccess = useGameAccess(gameId, shownOpponentId);

  // Explicit localPlayer null-check closes a window during reconnect where
  // `game` is present but `players[localPlayerId]` is not yet populated
  // (Event_GameStateChanged arrives after Event_GameJoined echo).
  const deckSelectOpen =
    game != null &&
    localPlayer != null &&
    !game.started &&
    !isSpectator &&
    !isJudge &&
    !localPlayer.properties.readyStart;

  const arrowSourceKey = pendingArrow
    ? makeCardKey(pendingArrow.sourcePlayerId, pendingArrow.sourceZone, pendingArrow.sourceCardId)
    : pendingAttach
      ? makeCardKey(pendingAttach.sourcePlayerId, pendingAttach.sourceZone, pendingAttach.sourceCardId)
      : arrowDrag
        ? makeCardKey(arrowDrag.sourcePlayerId, arrowDrag.sourceZone, arrowDrag.sourceCardId)
        : null;

  // Convert arrowDrag's viewport coords → board-relative coords for the SVG
  // preview line. Recomputed every render; cheap.
  const dragPreview = useMemo(() => {
    if (!arrowDrag || !arrowDrag.moved) {
      return null;
    }
    const boardRect = boardRef.current?.getBoundingClientRect();
    const sourceEl = cardRegistry.get(
      makeCardKey(arrowDrag.sourcePlayerId, arrowDrag.sourceZone, arrowDrag.sourceCardId),
    );
    if (!boardRect || !sourceEl) {
      return null;
    }
    const sourceRect = sourceEl.getBoundingClientRect();
    return {
      x1: sourceRect.left + sourceRect.width / 2 - boardRect.left,
      y1: sourceRect.top + sourceRect.height / 2 - boardRect.top,
      x2: arrowDrag.currentX - boardRect.left,
      y2: arrowDrag.currentY - boardRect.top,
      color: App.rgbaToCss(App.ArrowColor.RED),
    };
  }, [arrowDrag, cardRegistry]);

  const handleZoneClick = (playerId: number, zoneName: string) => {
    setZoneViews((prev) => {
      if (prev.some((v) => v.playerId === playerId && v.zoneName === zoneName)) {
        return prev;
      }
      return [...prev, { playerId, zoneName }];
    });
  };

  const handleCloseZoneView = (playerId: number, zoneName: string) => {
    setZoneViews((prev) =>
      prev.filter((v) => !(v.playerId === playerId && v.zoneName === zoneName)),
    );
  };

  const handleDragStart = (event: DragStartEvent) => {
    const data = event.active.data.current as
      | { card: Data.ServerInfo_Card }
      | undefined;
    setActiveCard(data?.card ?? null);
    // Starting a drag cancels any armed pending-arrow or pending-attach —
    // dnd-kit owns the pointer during the drag, matching desktop where the
    // arrow draw from context menu is aborted if the user grabs a card.
    if (pendingArrow) {
      setPendingArrow(null);
    }
    if (pendingAttach) {
      setPendingAttach(null);
    }
  };

  const handleDragEnd = (event: DragEndEvent) => {
    setActiveCard(null);
    if (!gameId || !event.over || !event.active.data.current) {
      return;
    }
    const source = event.active.data.current as {
      card: Data.ServerInfo_Card;
      sourcePlayerId: number;
      sourceZone: string;
    };
    const target = event.over.data.current as {
      targetPlayerId: number;
      targetZone: string;
      row?: number;
      attachTarget?: boolean;
      targetCardId?: number;
    };

    // Drop onto another card on the table → attach source to target.
    // Desktop's actAttach is only initiated from a table card, so source
    // must also be TABLE. Non-TABLE drops onto a table card fall through
    // to the normal moveCard branch (drop becomes "move to that row").
    if (
      target.attachTarget &&
      target.targetCardId != null &&
      source.sourceZone === App.ZoneName.TABLE
    ) {
      // Guard no-op self-drop (source === target).
      if (
        source.sourcePlayerId === target.targetPlayerId &&
        source.sourceZone === target.targetZone &&
        source.card.id === target.targetCardId
      ) {
        return;
      }
      webClient.request.game.attachCard(gameId, {
        startZone: source.sourceZone,
        cardId: source.card.id,
        targetPlayerId: target.targetPlayerId,
        targetZone: target.targetZone,
        targetCardId: target.targetCardId,
      });
      return;
    }

    const sameZone =
      source.sourcePlayerId === target.targetPlayerId &&
      source.sourceZone === target.targetZone;
    if (sameZone && source.sourceZone === App.ZoneName.TABLE && (source.card.y ?? 0) === (target.row ?? 0)) {
      return;
    }
    if (sameZone && source.sourceZone !== App.ZoneName.TABLE) {
      return;
    }

    webClient.request.game.moveCard(gameId, {
      startPlayerId: source.sourcePlayerId,
      startZone: source.sourceZone,
      cardsToMove: { card: [{ cardId: source.card.id }] },
      targetPlayerId: target.targetPlayerId,
      targetZone: target.targetZone,
      x: 0,
      y: target.row ?? 0,
      isReversed: false,
    });
  };

  const handleDragCancel = () => {
    setActiveCard(null);
  };

  const handleCardContextMenu = (
    sourcePlayerId: number,
    sourceZone: string,
    card: Data.ServerInfo_Card,
    event: React.MouseEvent,
  ) => {
    event.preventDefault();
    setZoneMenu(null);
    setCardMenu({
      card,
      sourcePlayerId,
      sourceZone,
      anchorPosition: { top: event.clientY, left: event.clientX },
    });
  };

  const handleZoneContextMenu = (
    playerId: number,
    zoneName: string,
    event: React.MouseEvent,
  ) => {
    if (playerId !== game?.localPlayerId) {
      return;
    }
    const supported =
      zoneName === App.ZoneName.DECK ||
      zoneName === App.ZoneName.GRAVE ||
      zoneName === App.ZoneName.EXILE;
    if (!supported) {
      return;
    }
    event.preventDefault();
    setCardMenu(null);
    setZoneMenu({
      playerId,
      zoneName,
      anchorPosition: { top: event.clientY, left: event.clientX },
    });
  };

  const handlePlayerContextMenu = (event: React.MouseEvent) => {
    if (gameId == null || isSpectator || localAccess.canAct === false) {
      return;
    }
    event.preventDefault();
    setPlayerMenu({ top: event.clientY, left: event.clientX });
  };

  const handleHandContextMenu = (event: React.MouseEvent) => {
    if (gameId == null || isSpectator || localAccess.canAct === false) {
      return;
    }
    event.preventDefault();
    setHandMenu({ top: event.clientY, left: event.clientX });
  };

  const handleCardClick = (
    ownerPlayerId: number,
    zone: string,
    card: Data.ServerInfo_Card,
  ) => {
    if (gameId == null) {
      return;
    }

    // Pending-attach (from CardContextMenu "Attach to card…") takes
    // precedence over pending-arrow because it was activated by a later menu
    // action. Click on the pending source to cancel.
    if (pendingAttach) {
      if (
        pendingAttach.sourcePlayerId === ownerPlayerId &&
        pendingAttach.sourceZone === zone &&
        pendingAttach.sourceCardId === card.id
      ) {
        setPendingAttach(null);
        return;
      }
      webClient.request.game.attachCard(gameId, {
        startZone: pendingAttach.sourceZone,
        cardId: pendingAttach.sourceCardId,
        targetPlayerId: ownerPlayerId,
        targetZone: zone,
        targetCardId: card.id,
      });
      setPendingAttach(null);
      return;
    }

    if (!pendingArrow) {
      return;
    }
    // Cancel if user re-clicks the pending source.
    if (
      pendingArrow.sourcePlayerId === ownerPlayerId &&
      pendingArrow.sourceZone === zone &&
      pendingArrow.sourceCardId === card.id
    ) {
      setPendingArrow(null);
      return;
    }
    // Desktop parity: arrow from local-hand → non-hand target auto-plays the
    // card (card_item.cpp:243-250). The server re-keys the moved card id, so
    // we resolve this as a play-card intent and drop the arrow command.
    if (
      pendingArrow.sourceZone === App.ZoneName.HAND &&
      pendingArrow.sourcePlayerId === game?.localPlayerId &&
      zone !== App.ZoneName.HAND
    ) {
      webClient.request.game.moveCard(gameId, {
        startPlayerId: pendingArrow.sourcePlayerId,
        startZone: pendingArrow.sourceZone,
        cardsToMove: { card: [{ cardId: pendingArrow.sourceCardId }] },
        targetPlayerId: pendingArrow.sourcePlayerId,
        targetZone: App.ZoneName.TABLE,
        x: 0,
        y: 0,
        isReversed: false,
      });
      setPendingArrow(null);
      return;
    }
    webClient.request.game.createArrow(gameId, {
      startPlayerId: pendingArrow.sourcePlayerId,
      startZone: pendingArrow.sourceZone,
      startCardId: pendingArrow.sourceCardId,
      targetPlayerId: ownerPlayerId,
      targetZone: zone,
      targetCardId: card.id,
      arrowColor: App.ArrowColor.RED,
    });
    setPendingArrow(null);
  };

  const handleCardDoubleClick = (
    sourceZone: string,
    card: Data.ServerInfo_Card,
  ) => {
    if (sourceZone !== App.ZoneName.TABLE || gameId == null) {
      return;
    }
    // Desktop's arrow drag owns the pointer while active; mirror that by
    // short-circuiting tap-toggle while a pending arrow/attach is armed.
    if (pendingArrow || pendingAttach) {
      return;
    }
    webClient.request.game.setCardAttr(gameId, {
      zone: sourceZone,
      cardId: card.id,
      attribute: Data.CardAttribute.AttrTapped,
      attrValue: card.tapped ? '0' : '1',
    });
  };

  const handleRequestSetPT = () => {
    const menu = cardMenu;
    if (!menu || gameId == null) {
      return;
    }
    setPrompt({
      title: 'Set power/toughness',
      label: 'P/T (e.g. 3/3)',
      initialValue: menu.card.pt ?? '',
      onSubmit: (value) => {
        webClient.request.game.setCardAttr(gameId, {
          zone: menu.sourceZone,
          cardId: menu.card.id,
          attribute: Data.CardAttribute.AttrPT,
          attrValue: value,
        });
        setPrompt(null);
      },
    });
  };

  const handleRequestSetAnnotation = () => {
    const menu = cardMenu;
    if (!menu || gameId == null) {
      return;
    }
    setPrompt({
      title: 'Set annotation',
      label: 'Annotation',
      initialValue: menu.card.annotation ?? '',
      onSubmit: (value) => {
        webClient.request.game.setCardAttr(gameId, {
          zone: menu.sourceZone,
          cardId: menu.card.id,
          attribute: Data.CardAttribute.AttrAnnotation,
          attrValue: value,
        });
        setPrompt(null);
      },
    });
  };

  const handleRequestSetCardCounter = () => {
    const menu = cardMenu;
    if (!menu || gameId == null) {
      return;
    }
    const existing = menu.card.counterList.find((c) => c.id === 0);
    setPrompt({
      title: 'Set card counter',
      label: 'Counter value',
      initialValue: String(existing?.value ?? 0),
      validate: (v) => (/^-?\d+$/.test(v) ? null : 'Enter an integer'),
      onSubmit: (value) => {
        webClient.request.game.setCardCounter(gameId, {
          zone: menu.sourceZone,
          cardId: menu.card.id,
          counterId: 0,
          counterValue: Number(value),
        });
        setPrompt(null);
      },
    });
  };

  const handleRequestDrawArrow = () => {
    const menu = cardMenu;
    if (!menu) {
      return;
    }
    setPendingArrow({
      sourcePlayerId: menu.sourcePlayerId,
      sourceZone: menu.sourceZone,
      sourceCardId: menu.card.id,
    });
  };

  const handleRequestAttach = () => {
    const menu = cardMenu;
    if (!menu) {
      return;
    }
    setPendingAttach({
      sourcePlayerId: menu.sourcePlayerId,
      sourceZone: menu.sourceZone,
      sourceCardId: menu.card.id,
    });
  };

  const handleRequestMoveToLibraryAt = () => {
    const menu = cardMenu;
    if (!menu || gameId == null || game == null) {
      return;
    }
    // Desktop prompts for a 1-indexed position into the library, then
    // internally subtracts 1 for the protocol's 0-indexed x-coordinate.
    setPrompt({
      title: 'Move to library at position',
      label: 'Position (1 = top)',
      initialValue: '1',
      validate: (v) => (/^[1-9]\d*$/.test(v) ? null : 'Enter a positive integer'),
      onSubmit: (value) => {
        webClient.request.game.moveCard(gameId, {
          startPlayerId: menu.sourcePlayerId,
          startZone: menu.sourceZone,
          cardsToMove: { card: [{ cardId: menu.card.id }] },
          targetPlayerId: game.localPlayerId,
          targetZone: App.ZoneName.DECK,
          x: Math.max(0, Number(value) - 1),
          y: 0,
          isReversed: false,
        });
        setPrompt(null);
      },
    });
  };

  const handleRequestDrawN = () => {
    if (gameId == null) {
      return;
    }
    setPrompt({
      title: 'Draw N cards',
      label: 'Number of cards',
      initialValue: '1',
      validate: (v) => (/^[1-9]\d*$/.test(v) ? null : 'Enter a positive integer'),
      onSubmit: (value) => {
        webClient.request.game.drawCards(gameId, { number: Number(value) });
        setPrompt(null);
      },
    });
  };

  const handleRequestDumpN = () => {
    if (gameId == null) {
      return;
    }
    setPrompt({
      title: 'Dump top N',
      label: 'Number of cards',
      initialValue: '1',
      validate: (v) => (/^[1-9]\d*$/.test(v) ? null : 'Enter a positive integer'),
      onSubmit: (value) => {
        webClient.request.game.dumpZone(gameId, {
          playerId: game!.localPlayerId,
          zoneName: App.ZoneName.DECK,
          numberCards: Number(value),
          isReversed: false,
        });
        setPrompt(null);
      },
    });
  };

  const handleRollDieSubmit = ({ sides, count }: { sides: number; count: number }) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.rollDie(gameId, { sides, count });
    setLastDieSides(sides);
    setLastDieCount(count);
    setRollDieOpen(false);
  };

  const handleCreateCounterSubmit = ({
    name,
    color,
  }: {
    name: string;
    color: { r: number; g: number; b: number; a: number };
  }) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.createCounter(gameId, {
      counterName: name,
      counterColor: color,
      radius: 1,
      value: 0,
    });
    setCreateCounterOpen(false);
  };

  const handleCreateTokenSubmit = (args: {
    name: string;
    color: string;
    pt: string;
    annotation: string;
    destroyOnZoneChange: boolean;
    faceDown: boolean;
  }) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.createToken(gameId, {
      zone: App.ZoneName.TABLE,
      cardName: args.name,
      color: args.color,
      pt: args.pt,
      annotation: args.annotation,
      destroyOnZoneChange: args.destroyOnZoneChange,
      x: 0,
      y: 0,
      faceDown: args.faceDown,
      targetCardId: -1,
    });
    setCreateTokenOpen(false);
  };

  const handleSideboardSubmit = (moveList: SideboardPlanMove[]) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.setSideboardPlan(gameId, { moveList });
    setSideboardOpen(false);
  };

  const handleToggleSideboardLock = (locked: boolean) => {
    if (gameId == null) {
      return;
    }
    webClient.request.game.setSideboardLock(gameId, { locked });
  };

  const handleRequestChooseMulligan = () => {
    if (gameId == null) {
      return;
    }
    // Desktop's DlgMulligan (player_actions.cpp actMulligan) accepts any
    // integer in [-handSize, handSize + deckSize]. 0 and negative values are
    // "relative to current hand size" — doMulligan computes
    // `handSize + number` before dispatching. Seeding with the configured
    // starting hand size (7) matches desktop's default.
    const handSize = localPlayer?.zones[App.ZoneName.HAND]?.cardCount ?? 0;
    const deckSize = localPlayer?.zones[App.ZoneName.DECK]?.cardCount ?? 0;
    const min = -handSize;
    const max = handSize + deckSize;
    setPrompt({
      title: 'Take mulligan',
      label: 'New hand size',
      initialValue: '7',
      helperText: '0 and lower are in comparison to current hand size.',
      validate: (v) => {
        if (!/^-?\d+$/.test(v)) {
          return 'Enter an integer.';
        }
        const n = Number(v);
        if (n < min || n > max) {
          return `Enter an integer between ${min} and ${max}.`;
        }
        return null;
      },
      onSubmit: (value) => {
        const input = Number(value);
        const resolved = input < 1 ? handSize + input : input;
        webClient.request.game.mulligan(gameId, { number: resolved });
        setPrompt(null);
      },
    });
  };

  const handleRequestRevealHand = () => {
    if (gameId == null) {
      return;
    }
    setRevealState({
      title: 'Reveal hand',
      zoneName: App.ZoneName.HAND,
      zoneLabel: 'Hand',
      showCountInput: false,
      defaultCount: 1,
      onSubmit: ({ targetPlayerId }) => {
        webClient.request.game.revealCards(gameId, {
          zoneName: App.ZoneName.HAND,
          playerId: targetPlayerId,
          topCards: -1,
        });
        setRevealState(null);
      },
    });
  };

  const handleRequestRevealRandom = () => {
    if (gameId == null) {
      return;
    }
    // Desktop's RANDOM_CARD_FROM_ZONE sentinel (-2); see
    // cockatrice/src/game/player/player_actions.h:47 and
    // actRevealRandomHandCard at player_actions.cpp:1705-1712.
    const RANDOM_CARD_FROM_ZONE = -2;
    setRevealState({
      title: 'Reveal random card',
      zoneName: App.ZoneName.HAND,
      zoneLabel: 'Hand (random)',
      showCountInput: false,
      defaultCount: 1,
      onSubmit: ({ targetPlayerId }) => {
        webClient.request.game.revealCards(gameId, {
          zoneName: App.ZoneName.HAND,
          cardId: [RANDOM_CARD_FROM_ZONE],
          playerId: targetPlayerId,
          topCards: -1,
        });
        setRevealState(null);
      },
    });
  };

  const handleRequestRevealTopN = () => {
    if (gameId == null) {
      return;
    }
    setRevealState({
      title: 'Reveal top N cards',
      zoneName: App.ZoneName.DECK,
      zoneLabel: 'Library',
      showCountInput: true,
      defaultCount: 1,
      onSubmit: ({ targetPlayerId, topCards }) => {
        webClient.request.game.revealCards(gameId, {
          zoneName: App.ZoneName.DECK,
          playerId: targetPlayerId,
          topCards,
        });
        setRevealState(null);
      },
    });
  };

  const handleRequestRevealZone = () => {
    if (gameId == null || zoneMenu == null) {
      return;
    }
    const { zoneName } = zoneMenu;
    const label =
      zoneName === App.ZoneName.GRAVE ? 'Graveyard' :
        zoneName === App.ZoneName.EXILE ? 'Exile' : zoneName;
    setRevealState({
      title: `Reveal ${label.toLowerCase()}`,
      zoneName,
      zoneLabel: label,
      showCountInput: false,
      defaultCount: 1,
      onSubmit: ({ targetPlayerId }) => {
        webClient.request.game.revealCards(gameId, {
          zoneName,
          playerId: targetPlayerId,
          topCards: -1,
        });
        setRevealState(null);
      },
    });
  };

  const revealPlayers = useMemo(() => {
    if (!game) {
      return [];
    }
    return Object.values(game.players)
      .filter((p) => p.properties.playerId !== game.localPlayerId)
      .map((p) => ({
        playerId: p.properties.playerId,
        name: p.properties.userInfo?.name ?? `p${p.properties.playerId}`,
      }));
  }, [game]);

  return (
    <Layout>
      <AuthGuard />
      <CardRegistryContext.Provider value={cardRegistry}>
        <DndContext
          sensors={sensors}
          onDragStart={handleDragStart}
          onDragEnd={handleDragEnd}
          onDragCancel={handleDragCancel}
        >
          <div className="game" data-testid="game-container">
            <PhaseBar gameId={gameId} />

            <div
              className="game__board"
              ref={boardRef}
              onMouseDown={handleBoardMouseDown}
            >
              <OpponentSelector
                opponents={opponents}
                selectedPlayerId={shownOpponentId}
                onSelect={setSelectedOpponentId}
              />

              {!game && (
                <div className="game__empty" data-testid="game-empty">
                  No active game. Join a game from a room to see the board.
                </div>
              )}

              {game && shownOpponentId != null && (
                <div
                  className={
                    'game__board-inner' +
                    (isRotated ? ' game__board-inner--rotated' : '')
                  }
                >
                  <PlayerBoard
                    gameId={gameId!}
                    playerId={shownOpponentId}
                    mirrored
                    canAct={opponentAccess.canAct}
                    canEditCounters={opponentAccess.canAct}
                    arrowSourceKey={arrowSourceKey}
                    onCardHover={setHoveredCard}
                    onCardClick={handleCardClick}
                    onCardContextMenu={(card, e) =>
                      handleCardContextMenu(shownOpponentId, App.ZoneName.TABLE, card, e)
                    }
                    onCardDoubleClick={(card) =>
                      handleCardDoubleClick(App.ZoneName.TABLE, card)
                    }
                    onZoneClick={handleZoneClick}
                    onZoneContextMenu={handleZoneContextMenu}
                  />
                  <StackStrip
                    gameId={gameId!}
                    entries={[
                      {
                        playerId: shownOpponentId,
                        name:
                          opponents.find((o) => o.playerId === shownOpponentId)?.name ??
                          `p${shownOpponentId}`,
                      },
                      {
                        playerId: game.localPlayerId,
                        name:
                          localPlayer?.properties.userInfo?.name ??
                          `p${game.localPlayerId}`,
                      },
                    ]}
                    onZoneClick={handleZoneClick}
                  />
                  <PlayerBoard
                    gameId={gameId!}
                    playerId={game.localPlayerId}
                    canAct={localAccess.canAct}
                    canEditCounters={localAccess.canAct}
                    arrowSourceKey={arrowSourceKey}
                    onCardHover={setHoveredCard}
                    onCardClick={handleCardClick}
                    onCardContextMenu={(card, e) =>
                      handleCardContextMenu(game.localPlayerId, App.ZoneName.TABLE, card, e)
                    }
                    onCardDoubleClick={(card) =>
                      handleCardDoubleClick(App.ZoneName.TABLE, card)
                    }
                    onZoneClick={handleZoneClick}
                    onZoneContextMenu={handleZoneContextMenu}
                    onRequestCreateCounter={() => setCreateCounterOpen(true)}
                    onPlayerContextMenu={handlePlayerContextMenu}
                  />
                  {localPlayer && (
                    <HandZone
                      gameId={gameId!}
                      playerId={game.localPlayerId}
                      canAct={localAccess.canAct}
                      arrowSourceKey={arrowSourceKey}
                      onCardHover={setHoveredCard}
                      onCardClick={handleCardClick}
                      onCardContextMenu={(card, e) =>
                        handleCardContextMenu(game.localPlayerId, App.ZoneName.HAND, card, e)
                      }
                      onZoneContextMenu={handleHandContextMenu}
                    />
                  )}
                </div>
              )}

              <GameArrowOverlay gameId={gameId} boardRef={boardRef} dragPreview={dragPreview} />
            </div>

            <RightPanel
              gameId={gameId}
              hoveredCard={hoveredCard}
              onRequestRollDie={() => setRollDieOpen(true)}
              onRequestConcede={() => setConcedeConfirm('concede')}
              onRequestUnconcede={() => setConcedeConfirm('unconcede')}
              onRequestGameInfo={() => setGameInfoOpen(true)}
              onToggleRotate90={() => setIsRotated((prev) => !prev)}
              isRotated={isRotated}
            />

            <DeckSelectDialog isOpen={deckSelectOpen} gameId={gameId} />

            {zoneViews.map((v, idx) => (
              <ZoneViewDialog
                key={`${v.playerId}-${v.zoneName}`}
                isOpen
                gameId={gameId}
                playerId={v.playerId}
                zoneName={v.zoneName}
                handleClose={() => handleCloseZoneView(v.playerId, v.zoneName)}
                initialPosition={{ x: 80 + idx * 36, y: 80 + idx * 36 }}
              />
            ))}

            <CardContextMenu
              isOpen={cardMenu != null}
              anchorPosition={cardMenu?.anchorPosition ?? null}
              gameId={gameId ?? 0}
              localPlayerId={game?.localPlayerId ?? null}
              card={cardMenu?.card ?? null}
              ownerPlayerId={cardMenu?.sourcePlayerId ?? null}
              sourceZone={cardMenu?.sourceZone ?? null}
              onClose={() => setCardMenu(null)}
              onRequestSetPT={handleRequestSetPT}
              onRequestSetAnnotation={handleRequestSetAnnotation}
              onRequestSetCounter={handleRequestSetCardCounter}
              onRequestDrawArrow={handleRequestDrawArrow}
              onRequestAttach={handleRequestAttach}
              onRequestMoveToLibraryAt={handleRequestMoveToLibraryAt}
            />

            <ZoneContextMenu
              isOpen={zoneMenu != null}
              anchorPosition={zoneMenu?.anchorPosition ?? null}
              gameId={gameId ?? 0}
              playerId={zoneMenu?.playerId ?? null}
              zoneName={zoneMenu?.zoneName ?? null}
              onClose={() => setZoneMenu(null)}
              onRequestDrawN={handleRequestDrawN}
              onRequestDumpN={handleRequestDumpN}
              onRequestRevealTopN={handleRequestRevealTopN}
              onRequestRevealZone={handleRequestRevealZone}
            />

            <PlayerContextMenu
              isOpen={playerMenu != null}
              anchorPosition={playerMenu}
              onClose={() => setPlayerMenu(null)}
              onRequestCreateToken={() => setCreateTokenOpen(true)}
              onRequestViewSideboard={() => setSideboardOpen(true)}
            />

            <HandContextMenu
              isOpen={handMenu != null}
              anchorPosition={handMenu}
              gameId={gameId ?? 0}
              handSize={localPlayer?.zones[App.ZoneName.HAND]?.cardCount ?? 0}
              onClose={() => setHandMenu(null)}
              onRequestChooseMulligan={handleRequestChooseMulligan}
              onRequestRevealHand={handleRequestRevealHand}
              onRequestRevealRandom={handleRequestRevealRandom}
            />

            {prompt && (
              <PromptDialog
                isOpen
                title={prompt.title}
                label={prompt.label}
                initialValue={prompt.initialValue}
                helperText={prompt.helperText}
                validate={prompt.validate}
                onSubmit={prompt.onSubmit}
                onCancel={() => setPrompt(null)}
              />
            )}

            <RollDieDialog
              isOpen={rollDieOpen}
              lastSides={lastDieSides}
              lastCount={lastDieCount}
              onSubmit={handleRollDieSubmit}
              onCancel={() => setRollDieOpen(false)}
            />

            <CreateCounterDialog
              isOpen={createCounterOpen}
              onSubmit={handleCreateCounterSubmit}
              onCancel={() => setCreateCounterOpen(false)}
            />

            <CreateTokenDialog
              isOpen={createTokenOpen}
              onSubmit={handleCreateTokenSubmit}
              onCancel={() => setCreateTokenOpen(false)}
            />

            <SideboardDialog
              isOpen={sideboardOpen}
              playerName={localPlayer?.properties.userInfo?.name ?? ''}
              deckCards={cardsFromZone(localPlayer?.zones[App.ZoneName.DECK])}
              sideboardCards={cardsFromZone(localPlayer?.zones[App.ZoneName.SIDEBOARD])}
              isLocked={localPlayer?.properties.sideboardLocked ?? false}
              onSubmit={handleSideboardSubmit}
              onCancel={() => setSideboardOpen(false)}
              onToggleLock={handleToggleSideboardLock}
            />

            {revealState && (
              <RevealCardsDialog
                isOpen
                title={revealState.title}
                zoneLabel={revealState.zoneLabel}
                showCountInput={revealState.showCountInput}
                defaultCount={revealState.defaultCount}
                players={revealPlayers}
                onSubmit={revealState.onSubmit}
                onCancel={() => setRevealState(null)}
              />
            )}

            <ConfirmDialog
              isOpen={concedeConfirm === 'concede'}
              title="Concede this game?"
              message="You'll stay seated as a spectator until you click Unconcede or Leave Game. Others will see you as conceded."
              confirmLabel="Concede"
              destructive
              onConfirm={() => {
                if (gameId != null) {
                  webClient.request.game.concede(gameId);
                }
                setConcedeConfirm(null);
              }}
              onCancel={() => setConcedeConfirm(null)}
            />

            <ConfirmDialog
              isOpen={concedeConfirm === 'unconcede'}
              title="Rejoin the game?"
              message="This undoes your concede and puts you back into the active player rotation."
              confirmLabel="Unconcede"
              onConfirm={() => {
                if (gameId != null) {
                  webClient.request.game.unconcede(gameId);
                }
                setConcedeConfirm(null);
              }}
              onCancel={() => setConcedeConfirm(null)}
            />

            <GameInfoDialog
              isOpen={gameInfoOpen}
              gameId={gameId}
              onClose={() => setGameInfoOpen(false)}
            />
          </div>

          <DragOverlay>
            {activeCard ? <CardDragOverlay card={activeCard} /> : null}
          </DragOverlay>
        </DndContext>
      </CardRegistryContext.Provider>
    </Layout>
  );
}

export default Game;
