import { useCallback, useState } from 'react';

import { DEFAULT_DIE_COUNT, DEFAULT_DIE_SIDES, type SideboardPlanMove } from '@app/dialogs';
import { useWebClient, type GameAccess } from '@app/hooks';
import { App, Data, type Enriched } from '@app/types';

export interface AnchorPosition {
  top: number;
  left: number;
}

export interface ZoneViewTarget {
  playerId: number;
  zoneName: string;
}

export interface CardMenuState {
  card: Data.ServerInfo_Card;
  sourcePlayerId: number;
  sourceZone: string;
  anchorPosition: AnchorPosition;
}

export interface ZoneMenuState {
  playerId: number;
  zoneName: string;
  anchorPosition: AnchorPosition;
}

export interface PromptState {
  title: string;
  label: string;
  initialValue?: string;
  helperText?: string;
  validate?: (value: string) => string | null;
  onSubmit: (value: string) => void;
}

export interface RevealState {
  title: string;
  zoneName: string;
  zoneLabel: string;
  showCountInput: boolean;
  defaultCount: number;
  onSubmit: (args: { targetPlayerId: number; topCards: number }) => void;
}

export type ConcedeConfirm = 'concede' | 'unconcede' | null;

export interface StartPendingSource {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
}

export interface GameDialogs {
  // Card/zone/player/hand menus
  cardMenu: CardMenuState | null;
  zoneMenu: ZoneMenuState | null;
  playerMenu: AnchorPosition | null;
  handMenu: AnchorPosition | null;
  closeCardMenu: () => void;
  closeZoneMenu: () => void;
  closePlayerMenu: () => void;
  closeHandMenu: () => void;
  handleCardContextMenu: (
    sourcePlayerId: number,
    sourceZone: string,
    card: Data.ServerInfo_Card,
    event: React.MouseEvent,
  ) => void;
  handleZoneContextMenu: (
    playerId: number,
    zoneName: string,
    event: React.MouseEvent,
  ) => void;
  handlePlayerContextMenu: (event: React.MouseEvent) => void;
  handleHandContextMenu: (event: React.MouseEvent) => void;

  // Zone-view dialog stack
  zoneViews: ZoneViewTarget[];
  handleZoneClick: (playerId: number, zoneName: string) => void;
  handleCloseZoneView: (playerId: number, zoneName: string) => void;

  // Prompt dialog
  prompt: PromptState | null;
  closePrompt: () => void;

  // Roll die dialog
  rollDieOpen: boolean;
  lastDieSides: number;
  lastDieCount: number;
  openRollDie: () => void;
  closeRollDie: () => void;
  handleRollDieSubmit: (args: { sides: number; count: number }) => void;

  // Counter / token / sideboard / game info / concede
  createCounterOpen: boolean;
  openCreateCounter: () => void;
  closeCreateCounter: () => void;
  handleCreateCounterSubmit: (args: {
    name: string;
    color: { r: number; g: number; b: number; a: number };
  }) => void;

  createTokenOpen: boolean;
  openCreateToken: () => void;
  closeCreateToken: () => void;
  handleCreateTokenSubmit: (args: {
    name: string;
    color: string;
    pt: string;
    annotation: string;
    destroyOnZoneChange: boolean;
    faceDown: boolean;
  }) => void;

  sideboardOpen: boolean;
  openSideboard: () => void;
  closeSideboard: () => void;
  handleSideboardSubmit: (moveList: SideboardPlanMove[]) => void;
  handleToggleSideboardLock: (locked: boolean) => void;

  gameInfoOpen: boolean;
  openGameInfo: () => void;
  closeGameInfo: () => void;

  concedeConfirm: ConcedeConfirm;
  openConcede: () => void;
  openUnconcede: () => void;
  closeConcedeConfirm: () => void;
  confirmConcede: () => void;
  confirmUnconcede: () => void;

  // Reveal-cards dialog
  revealState: RevealState | null;
  closeReveal: () => void;

  // Card context menu action handlers
  handleRequestSetPT: () => void;
  handleRequestSetAnnotation: () => void;
  handleRequestSetCardCounter: () => void;
  handleRequestDrawArrow: () => void;
  handleRequestAttach: () => void;
  handleRequestMoveToLibraryAt: () => void;

  // Zone context menu action handlers
  handleRequestDrawN: () => void;
  handleRequestDumpN: () => void;
  handleRequestRevealTopN: () => void;
  handleRequestRevealZone: () => void;

  // Hand context menu action handlers
  handleRequestChooseMulligan: () => void;
  handleRequestRevealHand: () => void;
  handleRequestRevealRandom: () => void;
}

export interface UseGameDialogsArgs {
  gameId: number | undefined;
  game: Enriched.GameEntry | undefined;
  localPlayer: Enriched.PlayerEntry | undefined;
  localAccess: GameAccess;
  isSpectator: boolean;
  startPendingArrow: (source: StartPendingSource) => void;
  startPendingAttach: (source: StartPendingSource) => void;
}

export function useGameDialogs({
  gameId,
  game,
  localPlayer,
  localAccess,
  isSpectator,
  startPendingArrow,
  startPendingAttach,
}: UseGameDialogsArgs): GameDialogs {
  const webClient = useWebClient();

  const [zoneViews, setZoneViews] = useState<ZoneViewTarget[]>([]);
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
  const [concedeConfirm, setConcedeConfirm] = useState<ConcedeConfirm>(null);
  const [gameInfoOpen, setGameInfoOpen] = useState(false);

  const handleZoneClick = useCallback((playerId: number, zoneName: string) => {
    setZoneViews((prev) => {
      if (prev.some((v) => v.playerId === playerId && v.zoneName === zoneName)) {
        return prev;
      }
      return [...prev, { playerId, zoneName }];
    });
  }, []);

  const handleCloseZoneView = useCallback((playerId: number, zoneName: string) => {
    setZoneViews((prev) =>
      prev.filter((v) => !(v.playerId === playerId && v.zoneName === zoneName)),
    );
  }, []);

  const handleCardContextMenu = useCallback(
    (
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
    },
    [],
  );

  const handleZoneContextMenu = useCallback(
    (playerId: number, zoneName: string, event: React.MouseEvent) => {
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
    },
    [game?.localPlayerId],
  );

  const handlePlayerContextMenu = useCallback(
    (event: React.MouseEvent) => {
      if (gameId == null || isSpectator || localAccess.canAct === false) {
        return;
      }
      event.preventDefault();
      setPlayerMenu({ top: event.clientY, left: event.clientX });
    },
    [gameId, isSpectator, localAccess.canAct],
  );

  const handleHandContextMenu = useCallback(
    (event: React.MouseEvent) => {
      if (gameId == null || isSpectator || localAccess.canAct === false) {
        return;
      }
      event.preventDefault();
      setHandMenu({ top: event.clientY, left: event.clientX });
    },
    [gameId, isSpectator, localAccess.canAct],
  );

  const handleRequestSetPT = useCallback(() => {
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
  }, [cardMenu, gameId, webClient]);

  const handleRequestSetAnnotation = useCallback(() => {
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
  }, [cardMenu, gameId, webClient]);

  const handleRequestSetCardCounter = useCallback(() => {
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
  }, [cardMenu, gameId, webClient]);

  const handleRequestDrawArrow = useCallback(() => {
    const menu = cardMenu;
    if (!menu) {
      return;
    }
    startPendingArrow({
      sourcePlayerId: menu.sourcePlayerId,
      sourceZone: menu.sourceZone,
      sourceCardId: menu.card.id,
    });
  }, [cardMenu, startPendingArrow]);

  const handleRequestAttach = useCallback(() => {
    const menu = cardMenu;
    if (!menu) {
      return;
    }
    startPendingAttach({
      sourcePlayerId: menu.sourcePlayerId,
      sourceZone: menu.sourceZone,
      sourceCardId: menu.card.id,
    });
  }, [cardMenu, startPendingAttach]);

  const handleRequestMoveToLibraryAt = useCallback(() => {
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
  }, [cardMenu, game, gameId, webClient]);

  const handleRequestDrawN = useCallback(() => {
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
  }, [gameId, webClient]);

  const handleRequestDumpN = useCallback(() => {
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
  }, [game, gameId, webClient]);

  const handleRollDieSubmit = useCallback(
    ({ sides, count }: { sides: number; count: number }) => {
      if (gameId == null) {
        return;
      }
      webClient.request.game.rollDie(gameId, { sides, count });
      setLastDieSides(sides);
      setLastDieCount(count);
      setRollDieOpen(false);
    },
    [gameId, webClient],
  );

  const handleCreateCounterSubmit = useCallback(
    ({
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
    },
    [gameId, webClient],
  );

  const handleCreateTokenSubmit = useCallback(
    (args: {
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
    },
    [gameId, webClient],
  );

  const handleSideboardSubmit = useCallback(
    (moveList: SideboardPlanMove[]) => {
      if (gameId == null) {
        return;
      }
      webClient.request.game.setSideboardPlan(gameId, { moveList });
      setSideboardOpen(false);
    },
    [gameId, webClient],
  );

  const handleToggleSideboardLock = useCallback(
    (locked: boolean) => {
      if (gameId == null) {
        return;
      }
      webClient.request.game.setSideboardLock(gameId, { locked });
    },
    [gameId, webClient],
  );

  const handleRequestChooseMulligan = useCallback(() => {
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
  }, [gameId, localPlayer, webClient]);

  const handleRequestRevealHand = useCallback(() => {
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
  }, [gameId, webClient]);

  const handleRequestRevealRandom = useCallback(() => {
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
  }, [gameId, webClient]);

  const handleRequestRevealTopN = useCallback(() => {
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
  }, [gameId, webClient]);

  const handleRequestRevealZone = useCallback(() => {
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
  }, [gameId, zoneMenu, webClient]);

  const confirmConcede = useCallback(() => {
    if (gameId != null) {
      webClient.request.game.concede(gameId);
    }
    setConcedeConfirm(null);
  }, [gameId, webClient]);

  const confirmUnconcede = useCallback(() => {
    if (gameId != null) {
      webClient.request.game.unconcede(gameId);
    }
    setConcedeConfirm(null);
  }, [gameId, webClient]);

  return {
    cardMenu,
    zoneMenu,
    playerMenu,
    handMenu,
    closeCardMenu: useCallback(() => setCardMenu(null), []),
    closeZoneMenu: useCallback(() => setZoneMenu(null), []),
    closePlayerMenu: useCallback(() => setPlayerMenu(null), []),
    closeHandMenu: useCallback(() => setHandMenu(null), []),
    handleCardContextMenu,
    handleZoneContextMenu,
    handlePlayerContextMenu,
    handleHandContextMenu,

    zoneViews,
    handleZoneClick,
    handleCloseZoneView,

    prompt,
    closePrompt: useCallback(() => setPrompt(null), []),

    rollDieOpen,
    lastDieSides,
    lastDieCount,
    openRollDie: useCallback(() => setRollDieOpen(true), []),
    closeRollDie: useCallback(() => setRollDieOpen(false), []),
    handleRollDieSubmit,

    createCounterOpen,
    openCreateCounter: useCallback(() => setCreateCounterOpen(true), []),
    closeCreateCounter: useCallback(() => setCreateCounterOpen(false), []),
    handleCreateCounterSubmit,

    createTokenOpen,
    openCreateToken: useCallback(() => setCreateTokenOpen(true), []),
    closeCreateToken: useCallback(() => setCreateTokenOpen(false), []),
    handleCreateTokenSubmit,

    sideboardOpen,
    openSideboard: useCallback(() => setSideboardOpen(true), []),
    closeSideboard: useCallback(() => setSideboardOpen(false), []),
    handleSideboardSubmit,
    handleToggleSideboardLock,

    gameInfoOpen,
    openGameInfo: useCallback(() => setGameInfoOpen(true), []),
    closeGameInfo: useCallback(() => setGameInfoOpen(false), []),

    concedeConfirm,
    openConcede: useCallback(() => setConcedeConfirm('concede'), []),
    openUnconcede: useCallback(() => setConcedeConfirm('unconcede'), []),
    closeConcedeConfirm: useCallback(() => setConcedeConfirm(null), []),
    confirmConcede,
    confirmUnconcede,

    revealState,
    closeReveal: useCallback(() => setRevealState(null), []),

    handleRequestSetPT,
    handleRequestSetAnnotation,
    handleRequestSetCardCounter,
    handleRequestDrawArrow,
    handleRequestAttach,
    handleRequestMoveToLibraryAt,
    handleRequestDrawN,
    handleRequestDumpN,
    handleRequestRevealTopN,
    handleRequestRevealZone,
    handleRequestChooseMulligan,
    handleRequestRevealHand,
    handleRequestRevealRandom,
  };
}
