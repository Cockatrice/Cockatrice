import {
  ArrowInfo,
  CardAttribute,
  CardCounterInfo,
  CardInfo,
  CounterInfo,
  PlayerInfo,
  PlayerProperties,
} from 'types';
import { create } from '@bufbuild/protobuf';
import { ServerInfo_CardSchema } from 'generated/proto/serverinfo_card_pb';
import { ServerInfo_CardCounterSchema } from 'generated/proto/serverinfo_cardcounter_pb';
import { GameAction } from './game.actions';
import { GameEntry, GameMessage, GamesState, PlayerEntry, ZoneEntry } from './game.interfaces';
import { Types } from './game.types';

// ── Helpers ──────────────────────────────────────────────────────────────────

function updateGame(state: GamesState, gameId: number, updates: Partial<GameEntry>): GamesState {
  const game = state.games[gameId];
  if (!game) {
    return state;
  }
  return {
    ...state,
    games: { ...state.games, [gameId]: { ...game, ...updates } },
  };
}

function updatePlayer(
  state: GamesState,
  gameId: number,
  playerId: number,
  updates: Partial<PlayerEntry>
): GamesState {
  const game = state.games[gameId];
  if (!game) {
    return state;
  }
  const player = game.players[playerId];
  if (!player) {
    return state;
  }
  return updateGame(state, gameId, {
    players: { ...game.players, [playerId]: { ...player, ...updates } },
  });
}

function updateZone(
  state: GamesState,
  gameId: number,
  playerId: number,
  zoneName: string,
  updates: Partial<ZoneEntry>
): GamesState {
  const game = state.games[gameId];
  if (!game) {
    return state;
  }
  const player = game.players[playerId];
  if (!player) {
    return state;
  }
  const zone = player.zones[zoneName];
  if (!zone) {
    return state;
  }
  return updatePlayer(state, gameId, playerId, {
    zones: { ...player.zones, [zoneName]: { ...zone, ...updates } },
  });
}

function removeGame(state: GamesState, gameId: number): GamesState {
  const games = { ...state.games };
  delete games[gameId];
  return { ...state, games };
}

/** Converts the proto PlayerInfo[] array into the keyed PlayerEntry map used in the store. */
function normalizePlayers(playerList: PlayerInfo[]): { [playerId: number]: PlayerEntry } {
  const players: { [playerId: number]: PlayerEntry } = {};
  for (const player of playerList) {
    const playerId = player.properties.playerId;

    const zones: { [zoneName: string]: ZoneEntry } = {};
    for (const zone of player.zoneList) {
      zones[zone.name] = {
        name: zone.name,
        type: zone.type,
        withCoords: zone.withCoords,
        cardCount: zone.cardCount,
        cards: [...zone.cardList],
        alwaysRevealTopCard: zone.alwaysRevealTopCard,
        alwaysLookAtTopCard: zone.alwaysLookAtTopCard,
      };
    }

    const counters: { [counterId: number]: CounterInfo } = {};
    for (const counter of player.counterList) {
      counters[counter.id] = counter;
    }

    const arrows: { [arrowId: number]: ArrowInfo } = {};
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
): CardInfo {
  return create(ServerInfo_CardSchema, {
    id,
    name,
    x,
    y,
    faceDown,
    tapped: false,
    attacking: false,
    color: '',
    pt: '',
    annotation: '',
    destroyOnZoneChange: false,
    doesntUntap: false,
    counterList: [],
    attachPlayerId: -1,
    attachZone: '',
    attachCardId: -1,
    providerId,
  });
}

// ── Initial state ─────────────────────────────────────────────────────────────

const initialState: GamesState = {
  games: {},
};

// ── Reducer ───────────────────────────────────────────────────────────────────

export const gamesReducer = (state: GamesState = initialState, action: GameAction): GamesState => {
  switch (action.type) {
    case Types.CLEAR_STORE: {
      return initialState;
    }

    case Types.GAME_JOINED: {
      return {
        ...state,
        games: { ...state.games, [action.gameId]: action.gameEntry },
      };
    }

    case Types.GAME_LEFT:
    case Types.GAME_CLOSED:
    case Types.KICKED: {
      return removeGame(state, action.gameId);
    }

    case Types.GAME_HOST_CHANGED: {
      return updateGame(state, action.gameId, { hostId: action.hostId });
    }

    case Types.GAME_STATE_CHANGED: {
      const { gameId, data } = action;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }

      const updates: Partial<GameEntry> = {};
      if (data.playerList?.length > 0) {
        updates.players = normalizePlayers(data.playerList);
      }
      if (data.gameStarted !== undefined && data.gameStarted !== null) {
        updates.started = data.gameStarted;
      }
      if (data.activePlayerId !== undefined && data.activePlayerId !== null) {
        updates.activePlayerId = data.activePlayerId;
      }
      if (data.activePhase !== undefined && data.activePhase !== null) {
        updates.activePhase = data.activePhase;
      }
      if (data.secondsElapsed !== undefined) {
        updates.secondsElapsed = data.secondsElapsed;
      }
      return updateGame(state, gameId, updates);
    }

    case Types.PLAYER_JOINED: {
      const { gameId, playerProperties } = action;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const newPlayer: PlayerEntry = {
        properties: playerProperties as PlayerProperties,
        deckList: '',
        zones: {},
        counters: {},
        arrows: {},
      };
      return updateGame(state, gameId, {
        players: { ...game.players, [playerProperties.playerId]: newPlayer },
      });
    }

    case Types.PLAYER_LEFT: {
      const { gameId, playerId } = action;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const players = { ...game.players };
      delete players[playerId];
      return updateGame(state, gameId, { players });
    }

    case Types.PLAYER_PROPERTIES_CHANGED: {
      return updatePlayer(state, action.gameId, action.playerId, {
        properties: action.properties,
      });
    }

    // ── Card manipulation ────────────────────────────────────────────────────

    case Types.CARD_MOVED: {
      const { gameId, playerId, data } = action;
      const {
        cardId,
        cardName,
        startPlayerId,
        startZone,
        position,
        targetPlayerId,
        targetZone,
        x,
        y,
        newCardId,
        faceDown,
        newCardProviderId,
      } = data;

      const game = state.games[gameId];
      if (!game) {
        return state;
      }

      const effectiveStartPlayerId = startPlayerId >= 0 ? startPlayerId : playerId;
      const sourcePlayer = game.players[effectiveStartPlayerId];
      const sourceZoneEntry = sourcePlayer?.zones[startZone];
      if (!sourcePlayer || !sourceZoneEntry) {
        return state;
      }

      // Locate card in source zone (by id for visible zones, by position for hidden)
      let removedCard: CardInfo | undefined;
      let newSourceCards: CardInfo[];
      if (cardId >= 0) {
        removedCard = sourceZoneEntry.cards.find(c => c.id === cardId);
        newSourceCards = sourceZoneEntry.cards.filter(c => c.id !== cardId);
      } else if (position >= 0 && position < sourceZoneEntry.cards.length) {
        removedCard = sourceZoneEntry.cards[position];
        newSourceCards = sourceZoneEntry.cards.filter((_, i) => i !== position);
      } else {
        // Hidden zone with unknown position — just decrement count
        newSourceCards = sourceZoneEntry.cards;
      }

      const effectiveNewId = newCardId >= 0 ? newCardId : (removedCard?.id ?? -1);
      const movedCard: CardInfo = removedCard
        ? {
          ...removedCard,
          id: effectiveNewId,
          name: cardName || removedCard.name,
          x,
          y,
          faceDown,
          providerId: newCardProviderId || removedCard.providerId,
        }
        : buildEmptyCard(effectiveNewId, cardName, x, y, faceDown, newCardProviderId ?? '');

      let newState = updateZone(state, gameId, effectiveStartPlayerId, startZone, {
        cards: newSourceCards,
        cardCount: Math.max(0, sourceZoneEntry.cardCount - 1),
      });

      const updatedGame = newState.games[gameId];
      const targetPlayer = updatedGame?.players[targetPlayerId];
      const targetZoneEntry = targetPlayer?.zones[targetZone];
      if (!targetPlayer || !targetZoneEntry) {
        return newState;
      }

      newState = updateZone(newState, gameId, targetPlayerId, targetZone, {
        cards: [...targetZoneEntry.cards, movedCard],
        cardCount: targetZoneEntry.cardCount + 1,
      });
      return newState;
    }

    case Types.CARD_FLIPPED: {
      const { gameId, playerId, data } = action;
      const { zoneName, cardId, cardName, faceDown, cardProviderId } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      const cardIdx = zone.cards.findIndex(c => c.id === cardId);
      if (cardIdx < 0) {
        return state;
      }

      const updatedCards = [...zone.cards];
      updatedCards[cardIdx] = {
        ...updatedCards[cardIdx],
        faceDown,
        name: cardName || updatedCards[cardIdx].name,
        providerId: cardProviderId || updatedCards[cardIdx].providerId,
      };
      return updateZone(state, gameId, playerId, zoneName, { cards: updatedCards });
    }

    case Types.CARD_DESTROYED: {
      const { gameId, playerId, data } = action;
      const { zoneName, cardId } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      return updateZone(state, gameId, playerId, zoneName, {
        cards: zone.cards.filter(c => c.id !== cardId),
        cardCount: Math.max(0, zone.cardCount - 1),
      });
    }

    case Types.CARD_ATTACHED: {
      const { gameId, playerId, data } = action;
      const { startZone, cardId, targetPlayerId, targetZone, targetCardId } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[startZone];
      if (!zone) {
        return state;
      }

      const cardIdx = zone.cards.findIndex(c => c.id === cardId);
      if (cardIdx < 0) {
        return state;
      }

      const updatedCards = [...zone.cards];
      updatedCards[cardIdx] = {
        ...updatedCards[cardIdx],
        attachPlayerId: targetPlayerId,
        attachZone: targetZone,
        attachCardId: targetCardId,
      };
      return updateZone(state, gameId, playerId, startZone, { cards: updatedCards });
    }

    case Types.TOKEN_CREATED: {
      const { gameId, playerId, data } = action;
      const {
        zoneName,
        cardId,
        cardName,
        color,
        pt,
        annotation,
        destroyOnZoneChange,
        x,
        y,
        cardProviderId,
        faceDown,
      } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      const newCard: CardInfo = create(ServerInfo_CardSchema, {
        id: cardId,
        name: cardName,
        x,
        y,
        faceDown,
        tapped: false,
        attacking: false,
        color,
        pt,
        annotation,
        destroyOnZoneChange,
        doesntUntap: false,
        counterList: [],
        attachPlayerId: -1,
        attachZone: '',
        attachCardId: -1,
        providerId: cardProviderId,
      });
      return updateZone(state, gameId, playerId, zoneName, {
        cards: [...zone.cards, newCard],
        cardCount: zone.cardCount + 1,
      });
    }

    case Types.CARD_ATTR_CHANGED: {
      const { gameId, playerId, data } = action;
      const { zoneName, cardId, attribute, attrValue } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      const cardIdx = zone.cards.findIndex(c => c.id === cardId);
      if (cardIdx < 0) {
        return state;
      }

      const attrPatch: Partial<CardInfo> = {};
      switch (attribute as CardAttribute) {
        case CardAttribute.AttrTapped: attrPatch.tapped = attrValue === '1'; break;
        case CardAttribute.AttrAttacking: attrPatch.attacking = attrValue === '1'; break;
        case CardAttribute.AttrFaceDown: attrPatch.faceDown = attrValue === '1'; break;
        case CardAttribute.AttrColor: attrPatch.color = attrValue; break;
        case CardAttribute.AttrPT: attrPatch.pt = attrValue; break;
        case CardAttribute.AttrAnnotation: attrPatch.annotation = attrValue; break;
        case CardAttribute.AttrDoesntUntap: attrPatch.doesntUntap = attrValue === '1'; break;
      }

      const updatedCards = [...zone.cards];
      updatedCards[cardIdx] = { ...updatedCards[cardIdx], ...attrPatch };
      return updateZone(state, gameId, playerId, zoneName, { cards: updatedCards });
    }

    case Types.CARD_COUNTER_CHANGED: {
      const { gameId, playerId, data } = action;
      const { zoneName, cardId, counterId, counterValue } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      const cardIdx = zone.cards.findIndex(c => c.id === cardId);
      if (cardIdx < 0) {
        return state;
      }

      const card = zone.cards[cardIdx];
      let newCounterList: CardCounterInfo[];
      if (counterValue <= 0) {
        newCounterList = card.counterList.filter(c => c.id !== counterId);
      } else {
        const existing = card.counterList.findIndex(c => c.id === counterId);
        newCounterList =
          existing >= 0
            ? card.counterList.map(c => (c.id === counterId ? { ...c, value: counterValue } : c))
            : [...card.counterList, create(ServerInfo_CardCounterSchema, { id: counterId, value: counterValue })];
      }

      const updatedCards = [...zone.cards];
      updatedCards[cardIdx] = { ...card, counterList: newCounterList };
      return updateZone(state, gameId, playerId, zoneName, { cards: updatedCards });
    }

    // ── Arrows ───────────────────────────────────────────────────────────────

    case Types.ARROW_CREATED: {
      const { gameId, playerId, data } = action;
      const { arrowInfo } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      return updatePlayer(state, gameId, playerId, {
        arrows: { ...player.arrows, [arrowInfo.id]: arrowInfo },
      });
    }

    case Types.ARROW_DELETED: {
      const { gameId, playerId, data } = action;
      const { arrowId } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const arrows = { ...player.arrows };
      delete arrows[arrowId];
      return updatePlayer(state, gameId, playerId, { arrows });
    }

    // ── Player counters ───────────────────────────────────────────────────────

    case Types.COUNTER_CREATED: {
      const { gameId, playerId, data } = action;
      const { counterInfo } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      return updatePlayer(state, gameId, playerId, {
        counters: { ...player.counters, [counterInfo.id]: counterInfo },
      });
    }

    case Types.COUNTER_SET: {
      const { gameId, playerId, data } = action;
      const { counterId, value } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const counter = player.counters[counterId];
      if (!counter) {
        return state;
      }
      return updatePlayer(state, gameId, playerId, {
        counters: { ...player.counters, [counterId]: { ...counter, count: value } },
      });
    }

    case Types.COUNTER_DELETED: {
      const { gameId, playerId, data } = action;
      const { counterId } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const counters = { ...player.counters };
      delete counters[counterId];
      return updatePlayer(state, gameId, playerId, { counters });
    }

    // ── Zone operations ───────────────────────────────────────────────────────

    case Types.CARDS_DRAWN: {
      const { gameId, playerId, data } = action;
      const { number: drawCount, cards } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }

      const deckZone = player.zones['deck'];
      const handZone = player.zones['hand'];
      if (!handZone) {
        return state;
      }

      // Decrement deck count for the drawing player
      let newState = deckZone
        ? updateZone(state, gameId, playerId, 'deck', {
          cardCount: Math.max(0, deckZone.cardCount - drawCount),
        })
        : state;

      // Append revealed cards to hand (cards array is empty for non-drawing players;
      // use drawCount for count math so all observers track the correct hand/deck size)
      const updatedHand = newState.games[gameId]!.players[playerId]!.zones['hand']!;
      return updateZone(newState, gameId, playerId, 'hand', {
        cards: [...updatedHand.cards, ...cards],
        cardCount: updatedHand.cardCount + drawCount,
      });
    }

    case Types.CARDS_REVEALED: {
      const { gameId, playerId, data } = action;
      const { zoneName, cards } = data;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const player = game.players[playerId];
      if (!player) {
        return state;
      }
      const zone = player.zones[zoneName];
      if (!zone) {
        return state;
      }

      // Merge revealed card data into existing zone cards (update existing, append new)
      const merged = [...zone.cards];
      for (const revealedCard of cards) {
        const idx = merged.findIndex(c => c.id === revealedCard.id);
        if (idx >= 0) {
          merged[idx] = { ...merged[idx], ...revealedCard };
        } else {
          merged.push(revealedCard);
        }
      }
      return updateZone(state, gameId, playerId, zoneName, { cards: merged });
    }

    case Types.ZONE_PROPERTIES_CHANGED: {
      const { gameId, playerId, data } = action;
      const { zoneName, alwaysRevealTopCard, alwaysLookAtTopCard } = data;
      const patch: Partial<ZoneEntry> = {};
      if (alwaysRevealTopCard !== undefined && alwaysRevealTopCard !== null) {
        patch.alwaysRevealTopCard = alwaysRevealTopCard;
      }
      if (alwaysLookAtTopCard !== undefined && alwaysLookAtTopCard !== null) {
        patch.alwaysLookAtTopCard = alwaysLookAtTopCard;
      }
      return updateZone(state, gameId, playerId, zoneName, patch);
    }

    // ── Turn / phase ──────────────────────────────────────────────────────────

    case Types.ACTIVE_PLAYER_SET: {
      return updateGame(state, action.gameId, { activePlayerId: action.activePlayerId });
    }

    case Types.ACTIVE_PHASE_SET: {
      return updateGame(state, action.gameId, { activePhase: action.phase });
    }

    case Types.TURN_REVERSED: {
      return updateGame(state, action.gameId, { reversed: action.reversed });
    }

    // ── Chat ──────────────────────────────────────────────────────────────────

    case Types.GAME_SAY: {
      const { gameId, playerId, message } = action;
      const game = state.games[gameId];
      if (!game) {
        return state;
      }
      const newMessage: GameMessage = { playerId, message, timeReceived: Date.now() };
      return updateGame(state, gameId, {
        messages: [...game.messages, newMessage],
      });
    }

    // ── Log-only events (state unchanged, future game log will use these) ─────
    case Types.ZONE_SHUFFLED:
    case Types.ZONE_DUMPED:
    case Types.DIE_ROLLED: {
      return state;
    }

    default:
      return state;
  }
};
