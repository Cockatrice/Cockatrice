import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { Data, Enriched } from '@app/types';
import { create, isFieldSet } from '@bufbuild/protobuf';
import { mergeSetFields } from '../common';
import { GamesState } from './game.interfaces';
import {
  diffPlayerProperties,
  formatActivePhaseSet,
  formatActivePlayerSet,
  formatArrowCreated,
  formatCardAttached,
  formatCardAttrChanged,
  formatCardCounterChanged,
  formatCardDestroyed,
  formatCardFlipped,
  formatCardMoved,
  formatCardsDrawn,
  formatCounterSet,
  formatDieRolled,
  formatGameStart,
  formatPlayerJoined,
  formatPropertyDiff,
  formatTokenCreated,
  formatTurnReversed,
  formatZoneDumped,
  formatZonePropertiesChanged,
  formatZoneShuffled,
} from './messageLog';

export const MAX_GAME_MESSAGES = 1000;

// Mirrors Event_Leave.LeaveReason values (1=OTHER, 2=USER_KICKED,
// 3=USER_LEFT, 4=USER_DISCONNECTED); kept in sync with desktop
// `GameEventHandler::getLeaveReason` in game_event_handler.cpp.
const LEAVE_REASON_MESSAGES: Record<number, string> = {
  1: 'reason unknown',
  2: 'kicked by game host or moderator',
  3: 'player left the game',
  4: 'player disconnected from server',
};

function formatLeaveMessage(playerName: string, reason: number): string {
  const reasonText = LEAVE_REASON_MESSAGES[reason] ?? LEAVE_REASON_MESSAGES[1];
  return `${playerName} has left the game (${reasonText}).`;
}

/** Timestamp source for event-log entries. Isolated so tests can mock it. */
function eventTimestamp(): number {
  return Date.now();
}

/** Push a formatted event-log message onto the game's message list, truncating to MAX_GAME_MESSAGES. */
function pushEventMessage(
  game: Enriched.GameEntry,
  playerId: number,
  message: string | null | undefined,
): void {
  if (!message) {
    return;
  }
  if (game.messages.length >= MAX_GAME_MESSAGES) {
    game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
  }
  game.messages.push({
    playerId,
    message,
    timeReceived: eventTimestamp(),
    kind: 'event',
  });
}

/** Converts the proto ServerInfo_Player[] array into the keyed PlayerEntry map. */
function normalizePlayers(playerList: Data.ServerInfo_Player[]): { [playerId: number]: Enriched.PlayerEntry } {
  const players: { [playerId: number]: Enriched.PlayerEntry } = {};
  for (const player of playerList) {
    const playerId = player.properties.playerId;

    const zones: { [zoneName: string]: Enriched.ZoneEntry } = {};
    for (const zone of player.zoneList) {
      const order: number[] = [];
      const byId: { [id: number]: Data.ServerInfo_Card } = {};
      for (const card of zone.cardList) {
        order.push(card.id);
        byId[card.id] = card;
      }
      zones[zone.name] = {
        name: zone.name as Enriched.ZoneNameValue,
        type: zone.type,
        withCoords: zone.withCoords,
        cardCount: zone.cardCount,
        order,
        byId,
        alwaysRevealTopCard: zone.alwaysRevealTopCard,
        alwaysLookAtTopCard: zone.alwaysLookAtTopCard,
      };
    }

    const counters: { [counterId: number]: Data.ServerInfo_Counter } = {};
    for (const counter of player.counterList) {
      counters[counter.id] = counter;
    }

    const arrows: { [arrowId: number]: Data.ServerInfo_Arrow } = {};
    for (const arrow of player.arrowList) {
      arrows[arrow.id] = arrow;
    }

    players[playerId] = {
      properties: player.properties,
      deckList: player.deckList,
      zones,
      counters,
      arrows,
    };
  }
  return players;
}

function buildEmptyCard(
  id: number,
  name: string,
  x: number,
  y: number,
  faceDown: boolean,
  providerId: string
): Data.ServerInfo_Card {
  return create(Data.ServerInfo_CardSchema, {
    id, name, x, y, faceDown,
    tapped: false, attacking: false, color: '', pt: '', annotation: '',
    destroyOnZoneChange: false, doesntUntap: false, counterList: [],
    attachPlayerId: -1, attachZone: '', attachCardId: -1, providerId,
  });
}

const initialState: GamesState = { games: {} };

export const gamesSlice = createSlice({
  name: 'games',
  initialState,
  reducers: {
    clearStore: () => initialState,

    gameJoined: (state, action: PayloadAction<{ data: Data.Event_GameJoined }>) => {
      const { data } = action.payload;
      const gameInfo = data.gameInfo;
      if (!gameInfo) {
        return;
      }
      state.games[gameInfo.gameId] = {
        info: gameInfo,
        hostId: data.hostId,
        localPlayerId: data.playerId,
        spectator: data.spectator,
        judge: data.judge,
        resuming: data.resuming,
        started: gameInfo.started,
        activePlayerId: -1,
        activePhase: -1,
        secondsElapsed: 0,
        reversed: false,
        players: {},
        messages: [],
      };
    },

    gameLeft: (state, action: PayloadAction<{ gameId: number }>) => {
      delete state.games[action.payload.gameId];
    },

    gameClosed: (state, action: PayloadAction<{ gameId: number }>) => {
      delete state.games[action.payload.gameId];
    },

    kicked: (state, action: PayloadAction<{ gameId: number }>) => {
      delete state.games[action.payload.gameId];
    },

    gameHostChanged: (state, action: PayloadAction<{ gameId: number; hostId: number }>) => {
      const { gameId, hostId } = action.payload;
      const game = state.games[gameId];
      if (game) {
        game.hostId = hostId;
      }
    },

    gameStateChanged: (state, action: PayloadAction<{ gameId: number; data: Data.Event_GameStateChanged }>) => {
      const { gameId, data } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }

      if (data.playerList?.length > 0) {
        game.players = normalizePlayers(data.playerList);
      }
      const wasStarted = game.started;
      if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.gameStarted)) {
        game.started = data.gameStarted;
      }
      if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.activePlayerId)) {
        game.activePlayerId = data.activePlayerId;
      }
      if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.activePhase)) {
        game.activePhase = data.activePhase;
      }
      if (isFieldSet(data, Data.Event_GameStateChangedSchema.field.secondsElapsed)) {
        game.secondsElapsed = data.secondsElapsed;
      }
      if (!wasStarted && game.started) {
        pushEventMessage(game, 0, formatGameStart());
      }
    },

    playerJoined: (state, action: PayloadAction<{ gameId: number; playerProperties: Data.ServerInfo_PlayerProperties }>) => {
      const { gameId, playerProperties } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      game.players[playerProperties.playerId] = {
        properties: playerProperties,
        deckList: '',
        zones: {},
        counters: {},
        arrows: {},
      };
      pushEventMessage(game, playerProperties.playerId, formatPlayerJoined(game, playerProperties.playerId));
    },

    playerLeft: (
      state,
      action: PayloadAction<{ gameId: number; playerId: number; reason: number; timeReceived: number }>,
    ) => {
      const { gameId, playerId, reason, timeReceived } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      const player = game.players[playerId];
      const playerName = player?.properties.userInfo?.name ?? 'Unknown player';

      if (game.messages.length >= MAX_GAME_MESSAGES) {
        game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
      }
      game.messages.push({
        playerId,
        message: formatLeaveMessage(playerName, reason),
        timeReceived,
        kind: 'event',
      });

      delete game.players[playerId];
    },

    playerPropertiesChanged: (
      state,
      action: PayloadAction<{ gameId: number; playerId: number; properties: Data.ServerInfo_PlayerProperties }>,
    ) => {
      const { gameId, playerId, properties } = action.payload;
      const game = state.games[gameId];
      const player = game?.players[playerId];
      if (!game || !player) {
        return;
      }
      const previous = { ...player.properties };
      mergeSetFields(Data.ServerInfo_PlayerPropertiesSchema, player.properties, properties);
      const diff = diffPlayerProperties(previous, player.properties);
      for (const msg of formatPropertyDiff(game, playerId, diff)) {
        pushEventMessage(game, playerId, msg);
      }
    },


    cardMoved: (
      state,
      action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_MoveCard }>,
    ) => {
      const { gameId, data } = action.payload;
      const {
        cardId, cardName, startPlayerId, startZone, position,
        targetPlayerId, targetZone, x, y, newCardId, faceDown, newCardProviderId,
      } = data;

      const game = state.games[gameId];
      if (!game) {
        return;
      }

      const sourcePlayer = game.players[startPlayerId];
      const sourceZone = sourcePlayer?.zones[startZone];
      if (!sourcePlayer || !sourceZone) {
        return;
      }

      const targetPlayer = game.players[targetPlayerId];
      const targetZoneEntry = targetPlayer?.zones[targetZone];
      if (!targetPlayer || !targetZoneEntry) {
        return;
      }

      let resolvedCardId = -1;
      if (cardId >= 0) {
        resolvedCardId = cardId;
      } else if (position >= 0 && position < sourceZone.order.length) {
        resolvedCardId = sourceZone.order[position];
      }

      // If the card can't be resolved and no newCardId is provided, the event
      // is malformed — bail out to avoid creating phantom cards with id -1.
      if (resolvedCardId < 0 && newCardId < 0) {
        return;
      }

      // Remove from source zone if the card was resolved to a known entry
      let removedCard: Data.ServerInfo_Card | undefined;
      if (resolvedCardId >= 0) {
        removedCard = sourceZone.byId[resolvedCardId];
        const idx = sourceZone.order.indexOf(resolvedCardId);
        if (idx >= 0) {
          sourceZone.order.splice(idx, 1);
        }
        delete sourceZone.byId[resolvedCardId];
      }
      sourceZone.cardCount = Math.max(0, sourceZone.cardCount - 1);

      const effectiveNewId = newCardId >= 0 ? newCardId : (removedCard?.id ?? resolvedCardId);
      const movedCard: Data.ServerInfo_Card = removedCard
        ? {
          ...removedCard, id: effectiveNewId, name: cardName || removedCard.name,
          x, y, faceDown, providerId: newCardProviderId || removedCard.providerId,
          counterList: [...removedCard.counterList],
        }
        : buildEmptyCard(effectiveNewId, cardName, x, y, faceDown, newCardProviderId ?? '');

      targetZoneEntry.order.push(movedCard.id);
      targetZoneEntry.byId[movedCard.id] = movedCard;
      targetZoneEntry.cardCount++;

      pushEventMessage(
        game,
        action.payload.playerId,
        formatCardMoved(game, action.payload.playerId, data, {
          resolvedCardName: removedCard?.name ?? '',
        }),
      );
    },

    cardFlipped: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_FlipCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, cardName, faceDown, cardProviderId } = data;
      const game = state.games[gameId];
      const card = game?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!game || !card) {
        return;
      }
      const previousName = card.name;
      card.faceDown = faceDown;
      if (cardName) {
        card.name = cardName;
      }
      if (cardProviderId) {
        card.providerId = cardProviderId;
      }
      pushEventMessage(game, playerId, formatCardFlipped(game, playerId, data, previousName));
    },

    cardDestroyed: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DestroyCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId } = data;
      const game = state.games[gameId];
      const zone = game?.players[playerId]?.zones[zoneName];
      if (!game || !zone) {
        return;
      }
      const destroyedName = zone.byId[cardId]?.name;
      const idx = zone.order.indexOf(cardId);
      if (idx >= 0) {
        zone.order.splice(idx, 1);
      }
      delete zone.byId[cardId];
      zone.cardCount = Math.max(0, zone.cardCount - 1);
      pushEventMessage(game, playerId, formatCardDestroyed(game, playerId, destroyedName));
    },

    cardAttached: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_AttachCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { startZone, cardId, targetPlayerId, targetZone, targetCardId } = data;
      const game = state.games[gameId];
      const card = game?.players[playerId]?.zones[startZone]?.byId[cardId];
      if (!game || !card) {
        return;
      }
      const sourceCardName = card.name;
      card.attachPlayerId = targetPlayerId;
      card.attachZone = targetZone;
      card.attachCardId = targetCardId;
      pushEventMessage(game, playerId, formatCardAttached(game, playerId, data, sourceCardName));
    },

    tokenCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateToken }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, cardName, color, pt, annotation, destroyOnZoneChange, x, y, cardProviderId, faceDown } = data;
      const game = state.games[gameId];
      const zone = game?.players[playerId]?.zones[zoneName];
      if (!game || !zone) {
        return;
      }
      const newCard = create(Data.ServerInfo_CardSchema, {
        id: cardId, name: cardName, x, y, faceDown,
        tapped: false, attacking: false, color, pt, annotation, destroyOnZoneChange,
        doesntUntap: false, counterList: [],
        attachPlayerId: -1, attachZone: '', attachCardId: -1, providerId: cardProviderId,
      });
      zone.order.push(newCard.id);
      zone.byId[newCard.id] = newCard;
      zone.cardCount++;
      pushEventMessage(game, playerId, formatTokenCreated(game, playerId, data));
    },

    cardAttrChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCardAttr }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, attribute, attrValue } = data;
      const game = state.games[gameId];
      const card = game?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!game || !card) {
        return;
      }
      const cardName = card.name;
      switch (attribute as Data.CardAttribute) {
        case Data.CardAttribute.AttrTapped: card.tapped = attrValue === '1'; break;
        case Data.CardAttribute.AttrAttacking: card.attacking = attrValue === '1'; break;
        case Data.CardAttribute.AttrFaceDown: card.faceDown = attrValue === '1'; break;
        case Data.CardAttribute.AttrColor: card.color = attrValue; break;
        case Data.CardAttribute.AttrPT: card.pt = attrValue; break;
        case Data.CardAttribute.AttrAnnotation: card.annotation = attrValue; break;
        case Data.CardAttribute.AttrDoesntUntap: card.doesntUntap = attrValue === '1'; break;
      }
      pushEventMessage(game, playerId, formatCardAttrChanged(game, playerId, data, cardName));
    },

    cardCounterChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCardCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, counterId, counterValue } = data;
      const game = state.games[gameId];
      const card = game?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!game || !card) {
        return;
      }
      const cardName = card.name;
      const previousValue = card.counterList.find(c => c.id === counterId)?.value ?? 0;
      if (counterValue <= 0) {
        card.counterList = card.counterList.filter(c => c.id !== counterId);
      } else {
        const idx = card.counterList.findIndex(c => c.id === counterId);
        if (idx >= 0) {
          card.counterList[idx] = { ...card.counterList[idx], value: counterValue };
        } else {
          card.counterList.push(create(Data.ServerInfo_CardCounterSchema, { id: counterId, value: counterValue }));
        }
      }
      pushEventMessage(
        game,
        playerId,
        formatCardCounterChanged(game, playerId, data, cardName, previousValue),
      );
    },


    arrowCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateArrow }>) => {
      const { gameId, playerId, data } = action.payload;
      const game = state.games[gameId];
      const player = game?.players[playerId];
      if (!game || !player || !data.arrowInfo) {
        return;
      }
      player.arrows[data.arrowInfo.id] = { ...data.arrowInfo };
      pushEventMessage(game, playerId, formatArrowCreated(game, playerId, data.arrowInfo));
    },

    arrowDeleted: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DeleteArrow }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        delete player.arrows[data.arrowId];
      }
    },


    counterCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player && data.counterInfo) {
        player.counters[data.counterInfo.id] = { ...data.counterInfo };
      }
    },

    counterSet: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const game = state.games[gameId];
      const counter = game?.players[playerId]?.counters[data.counterId];
      if (!game || !counter) {
        return;
      }
      const previousValue = counter.count;
      counter.count = data.value;
      pushEventMessage(
        game,
        playerId,
        formatCounterSet(game, playerId, data, counter.name, previousValue),
      );
    },

    counterDeleted: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DelCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        delete player.counters[data.counterId];
      }
    },


    cardsDrawn: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DrawCards }>) => {
      const { gameId, playerId, data } = action.payload;
      const { number: drawCount, cards } = data;
      const game = state.games[gameId];
      const player = game?.players[playerId];
      if (!game || !player) {
        return;
      }

      const deckZone = player.zones[Enriched.ZoneName.DECK];
      const handZone = player.zones[Enriched.ZoneName.HAND];
      if (!handZone) {
        return;
      }

      if (deckZone) {
        deckZone.cardCount = Math.max(0, deckZone.cardCount - drawCount);
      }

      for (const card of cards) {
        handZone.order.push(card.id);
        handZone.byId[card.id] = card;
      }
      handZone.cardCount += drawCount;

      pushEventMessage(game, playerId, formatCardsDrawn(game, playerId, drawCount));
    },

    cardsRevealed: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_RevealCards }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cards } = data;
      const zone = state.games[gameId]?.players[playerId]?.zones[zoneName];
      if (!zone) {
        return;
      }

      for (const revealedCard of cards) {
        if (!zone.byId[revealedCard.id]) {
          zone.order.push(revealedCard.id);
        }
        zone.byId[revealedCard.id] = { ...revealedCard, counterList: [...revealedCard.counterList] };
      }
    },

    zonePropertiesChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_ChangeZoneProperties }>) => {
      const { gameId, playerId, data } = action.payload;
      const game = state.games[gameId];
      const zone = game?.players[playerId]?.zones[data.zoneName];
      if (!game || !zone) {
        return;
      }
      if (isFieldSet(data, Data.Event_ChangeZonePropertiesSchema.field.alwaysRevealTopCard)) {
        zone.alwaysRevealTopCard = data.alwaysRevealTopCard;
      }
      if (isFieldSet(data, Data.Event_ChangeZonePropertiesSchema.field.alwaysLookAtTopCard)) {
        zone.alwaysLookAtTopCard = data.alwaysLookAtTopCard;
      }
      pushEventMessage(game, playerId, formatZonePropertiesChanged(game, playerId, data));
    },


    activePlayerSet: (state, action: PayloadAction<{ gameId: number; activePlayerId: number }>) => {
      const game = state.games[action.payload.gameId];
      if (!game) {
        return;
      }
      const previous = game.activePlayerId;
      game.activePlayerId = action.payload.activePlayerId;
      if (previous !== action.payload.activePlayerId) {
        pushEventMessage(game, action.payload.activePlayerId, formatActivePlayerSet(game, action.payload.activePlayerId));
      }
    },

    activePhaseSet: (state, action: PayloadAction<{ gameId: number; phase: number }>) => {
      const game = state.games[action.payload.gameId];
      if (!game) {
        return;
      }
      const previous = game.activePhase;
      game.activePhase = action.payload.phase;
      if (previous !== action.payload.phase && game.started) {
        pushEventMessage(game, 0, formatActivePhaseSet(action.payload.phase));
      }
    },

    turnReversed: (state, action: PayloadAction<{ gameId: number; reversed: boolean }>) => {
      const game = state.games[action.payload.gameId];
      if (!game) {
        return;
      }
      game.reversed = action.payload.reversed;
      pushEventMessage(game, game.activePlayerId, formatTurnReversed(game, game.activePlayerId, action.payload.reversed));
    },


    gameSay: (state, action: PayloadAction<{ gameId: number; playerId: number; message: string; timeReceived: number }>) => {
      const { gameId, playerId, message, timeReceived } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      if (game.messages.length >= MAX_GAME_MESSAGES) {
        game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
      }
      game.messages.push({ playerId, message, timeReceived, kind: 'chat' });
    },

    // Logged-only actions: no state mutation but an event-log entry.
    zoneShuffled: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_Shuffle }>) => {
      const { gameId, playerId } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      pushEventMessage(game, playerId, formatZoneShuffled(game, playerId));
    },
    zoneDumped: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DumpZone }>) => {
      const { gameId, playerId, data } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      pushEventMessage(game, playerId, formatZoneDumped(game, playerId, data));
    },
    dieRolled: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_RollDie }>) => {
      const { gameId, playerId, data } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      pushEventMessage(game, playerId, formatDieRolled(game, playerId, data));
    },
  },
});

export const gamesReducer = gamesSlice.reducer;
