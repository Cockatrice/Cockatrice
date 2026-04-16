import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { Data, Enriched } from '@app/types';
import { create } from '@bufbuild/protobuf';
import { GamesState } from './game.interfaces';

export const MAX_GAME_MESSAGES = 1000;

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
        name: zone.name,
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
      if (data.gameStarted !== undefined && data.gameStarted !== null) {
        game.started = data.gameStarted;
      }
      if (data.activePlayerId !== undefined && data.activePlayerId !== null) {
        game.activePlayerId = data.activePlayerId;
      }
      if (data.activePhase !== undefined && data.activePhase !== null) {
        game.activePhase = data.activePhase;
      }
      if (data.secondsElapsed !== undefined) {
        game.secondsElapsed = data.secondsElapsed;
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
    },

    playerLeft: (state, action: PayloadAction<{ gameId: number; playerId: number }>) => {
      const { gameId, playerId } = action.payload;
      const game = state.games[gameId];
      if (game) {
        delete game.players[playerId];
      }
    },

    playerPropertiesChanged: (
      state,
      action: PayloadAction<{ gameId: number; playerId: number; properties: Data.ServerInfo_PlayerProperties }>,
    ) => {
      const { gameId, playerId, properties } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        player.properties = properties;
      }
    },

    // ── Card manipulation ────────────────────────────────────────────────────

    cardMoved: (
      state,
      action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_MoveCard }>,
    ) => {
      const { gameId, playerId, data } = action.payload;
      const {
        cardId, cardName, startPlayerId, startZone, position,
        targetPlayerId, targetZone, x, y, newCardId, faceDown, newCardProviderId,
      } = data;

      const game = state.games[gameId];
      if (!game) {
        return;
      }

      const effectiveStartPlayerId = startPlayerId >= 0 ? startPlayerId : playerId;
      const sourcePlayer = game.players[effectiveStartPlayerId];
      const sourceZone = sourcePlayer?.zones[startZone];
      if (!sourcePlayer || !sourceZone) {
        return;
      }

      let resolvedCardId = -1;
      if (cardId >= 0) {
        resolvedCardId = cardId;
      } else if (position >= 0 && position < sourceZone.order.length) {
        resolvedCardId = sourceZone.order[position];
      }

      const removedCard: Data.ServerInfo_Card | undefined =
        resolvedCardId >= 0 ? sourceZone.byId[resolvedCardId] : undefined;

      if (resolvedCardId >= 0) {
        const idx = sourceZone.order.indexOf(resolvedCardId);
        if (idx >= 0) {
          sourceZone.order.splice(idx, 1);
        }
        delete sourceZone.byId[resolvedCardId];
      }
      sourceZone.cardCount = Math.max(0, sourceZone.cardCount - 1);

      const effectiveNewId = newCardId >= 0 ? newCardId : (removedCard?.id ?? -1);
      const movedCard: Data.ServerInfo_Card = removedCard
        ? {
          ...removedCard, id: effectiveNewId, name: cardName || removedCard.name,
          x, y, faceDown, providerId: newCardProviderId || removedCard.providerId,
        }
        : buildEmptyCard(effectiveNewId, cardName, x, y, faceDown, newCardProviderId ?? '');

      const targetPlayer = game.players[targetPlayerId];
      const targetZoneEntry = targetPlayer?.zones[targetZone];
      if (!targetPlayer || !targetZoneEntry) {
        return;
      }

      targetZoneEntry.order.push(movedCard.id);
      targetZoneEntry.byId[movedCard.id] = movedCard;
      targetZoneEntry.cardCount++;
    },

    cardFlipped: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_FlipCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, cardName, faceDown, cardProviderId } = data;
      const card = state.games[gameId]?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!card) {
        return;
      }
      card.faceDown = faceDown;
      if (cardName) {
        card.name = cardName;
      }
      if (cardProviderId) {
        card.providerId = cardProviderId;
      }
    },

    cardDestroyed: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DestroyCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId } = data;
      const zone = state.games[gameId]?.players[playerId]?.zones[zoneName];
      if (!zone) {
        return;
      }
      const idx = zone.order.indexOf(cardId);
      if (idx >= 0) {
        zone.order.splice(idx, 1);
      }
      delete zone.byId[cardId];
      zone.cardCount = Math.max(0, zone.cardCount - 1);
    },

    cardAttached: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_AttachCard }>) => {
      const { gameId, playerId, data } = action.payload;
      const { startZone, cardId, targetPlayerId, targetZone, targetCardId } = data;
      const card = state.games[gameId]?.players[playerId]?.zones[startZone]?.byId[cardId];
      if (!card) {
        return;
      }
      card.attachPlayerId = targetPlayerId;
      card.attachZone = targetZone;
      card.attachCardId = targetCardId;
    },

    tokenCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateToken }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, cardName, color, pt, annotation, destroyOnZoneChange, x, y, cardProviderId, faceDown } = data;
      const zone = state.games[gameId]?.players[playerId]?.zones[zoneName];
      if (!zone) {
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
    },

    cardAttrChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCardAttr }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, attribute, attrValue } = data;
      const card = state.games[gameId]?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!card) {
        return;
      }
      switch (attribute as Data.CardAttribute) {
        case Data.CardAttribute.AttrTapped: card.tapped = attrValue === '1'; break;
        case Data.CardAttribute.AttrAttacking: card.attacking = attrValue === '1'; break;
        case Data.CardAttribute.AttrFaceDown: card.faceDown = attrValue === '1'; break;
        case Data.CardAttribute.AttrColor: card.color = attrValue; break;
        case Data.CardAttribute.AttrPT: card.pt = attrValue; break;
        case Data.CardAttribute.AttrAnnotation: card.annotation = attrValue; break;
        case Data.CardAttribute.AttrDoesntUntap: card.doesntUntap = attrValue === '1'; break;
      }
    },

    cardCounterChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCardCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cardId, counterId, counterValue } = data;
      const card = state.games[gameId]?.players[playerId]?.zones[zoneName]?.byId[cardId];
      if (!card) {
        return;
      }
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
    },

    // ── Arrows ───────────────────────────────────────────────────────────────

    arrowCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateArrow }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        player.arrows[data.arrowInfo.id] = data.arrowInfo;
      }
    },

    arrowDeleted: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DeleteArrow }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        delete player.arrows[data.arrowId];
      }
    },

    // ── Player counters ───────────────────────────────────────────────────────

    counterCreated: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_CreateCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        player.counters[data.counterInfo.id] = data.counterInfo;
      }
    },

    counterSet: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_SetCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const counter = state.games[gameId]?.players[playerId]?.counters[data.counterId];
      if (counter) {
        counter.count = data.value;
      }
    },

    counterDeleted: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DelCounter }>) => {
      const { gameId, playerId, data } = action.payload;
      const player = state.games[gameId]?.players[playerId];
      if (player) {
        delete player.counters[data.counterId];
      }
    },

    // ── Zone operations ───────────────────────────────────────────────────────

    cardsDrawn: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DrawCards }>) => {
      const { gameId, playerId, data } = action.payload;
      const { number: drawCount, cards } = data;
      const player = state.games[gameId]?.players[playerId];
      if (!player) {
        return;
      }

      const deckZone = player.zones['deck'];
      const handZone = player.zones['hand'];
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
    },

    cardsRevealed: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_RevealCards }>) => {
      const { gameId, playerId, data } = action.payload;
      const { zoneName, cards } = data;
      const zone = state.games[gameId]?.players[playerId]?.zones[zoneName];
      if (!zone) {
        return;
      }

      for (const revealedCard of cards) {
        if (zone.byId[revealedCard.id]) {
          Object.assign(zone.byId[revealedCard.id], revealedCard);
        } else {
          zone.order.push(revealedCard.id);
          zone.byId[revealedCard.id] = revealedCard;
        }
      }
    },

    zonePropertiesChanged: (state, action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_ChangeZoneProperties }>) => {
      const { gameId, playerId, data } = action.payload;
      const zone = state.games[gameId]?.players[playerId]?.zones[data.zoneName];
      if (!zone) {
        return;
      }
      if (data.alwaysRevealTopCard !== undefined && data.alwaysRevealTopCard !== null) {
        zone.alwaysRevealTopCard = data.alwaysRevealTopCard;
      }
      if (data.alwaysLookAtTopCard !== undefined && data.alwaysLookAtTopCard !== null) {
        zone.alwaysLookAtTopCard = data.alwaysLookAtTopCard;
      }
    },

    // ── Turn / phase ──────────────────────────────────────────────────────────

    activePlayerSet: (state, action: PayloadAction<{ gameId: number; activePlayerId: number }>) => {
      const game = state.games[action.payload.gameId];
      if (game) {
        game.activePlayerId = action.payload.activePlayerId;
      }
    },

    activePhaseSet: (state, action: PayloadAction<{ gameId: number; phase: number }>) => {
      const game = state.games[action.payload.gameId];
      if (game) {
        game.activePhase = action.payload.phase;
      }
    },

    turnReversed: (state, action: PayloadAction<{ gameId: number; reversed: boolean }>) => {
      const game = state.games[action.payload.gameId];
      if (game) {
        game.reversed = action.payload.reversed;
      }
    },

    // ── Chat ──────────────────────────────────────────────────────────────────

    gameSay: (state, action: PayloadAction<{ gameId: number; playerId: number; message: string }>) => {
      const { gameId, playerId, message } = action.payload;
      const game = state.games[gameId];
      if (!game) {
        return;
      }
      if (game.messages.length >= MAX_GAME_MESSAGES) {
        game.messages = game.messages.slice(game.messages.length - MAX_GAME_MESSAGES + 1);
      }
      game.messages.push({ playerId, message, timeReceived: Date.now() });
    },

    // ── Log-only events ─────────────────────────────────────────────────────
    zoneShuffled: (_state, _action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_Shuffle }>) => {},
    zoneDumped: (_state, _action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_DumpZone }>) => {},
    dieRolled: (_state, _action: PayloadAction<{ gameId: number; playerId: number; data: Data.Event_RollDie }>) => {},
  },
});

export const gamesReducer = gamesSlice.reducer;
