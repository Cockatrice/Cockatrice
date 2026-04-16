vi.mock('../../WebClient', () => ({
  WebClient: {
    instance: {
      response: {
        game: {
          gameStateChanged: vi.fn(),
          playerJoined: vi.fn(),
          playerLeft: vi.fn(),
          playerPropertiesChanged: vi.fn(),
          gameClosed: vi.fn(),
          gameHostChanged: vi.fn(),
          kicked: vi.fn(),
          gameSay: vi.fn(),
          cardMoved: vi.fn(),
          cardFlipped: vi.fn(),
          cardDestroyed: vi.fn(),
          cardAttached: vi.fn(),
          tokenCreated: vi.fn(),
          cardAttrChanged: vi.fn(),
          cardCounterChanged: vi.fn(),
          arrowCreated: vi.fn(),
          arrowDeleted: vi.fn(),
          counterCreated: vi.fn(),
          counterSet: vi.fn(),
          counterDeleted: vi.fn(),
          cardsDrawn: vi.fn(),
          cardsRevealed: vi.fn(),
          zoneShuffled: vi.fn(),
          dieRolled: vi.fn(),
          activePlayerSet: vi.fn(),
          activePhaseSet: vi.fn(),
          turnReversed: vi.fn(),
          zoneDumped: vi.fn(),
          zonePropertiesChanged: vi.fn(),
        },
      },
    },
  },
}));

import { useWebClientCleanup } from '../../__mocks__/helpers';
import { create } from '@bufbuild/protobuf';
import {
  Event_AttachCardSchema,
  Event_ChangeZonePropertiesSchema,
  Event_CreateArrowSchema,
  Event_CreateCounterSchema,
  Event_CreateTokenSchema,
  Event_DelCounterSchema,
  Event_DeleteArrowSchema,
  Event_DestroyCardSchema,
  Event_DrawCardsSchema,
  Event_DumpZoneSchema,
  Event_FlipCardSchema,
  Event_GameSaySchema,
  Event_GameStateChangedSchema,
  Event_MoveCardSchema,
  Event_RevealCardsSchema,
  Event_ReverseTurnSchema,
  Event_RollDieSchema,
  Event_SetActivePhaseSchema,
  Event_SetActivePlayerSchema,
  Event_SetCardAttrSchema,
  Event_SetCardCounterSchema,
  Event_SetCounterSchema,
  Event_ShuffleSchema,
  ServerInfo_PlayerPropertiesSchema,
} from '@app/generated';
import { WebClient } from '../../WebClient';

import { attachCard } from './attachCard';
import { changeZoneProperties } from './changeZoneProperties';
import { createArrow } from './createArrow';
import { createCounter } from './createCounter';
import { createToken } from './createToken';
import { delCounter } from './delCounter';
import { deleteArrow } from './deleteArrow';
import { destroyCard } from './destroyCard';
import { drawCards } from './drawCards';
import { dumpZone } from './dumpZone';
import { flipCard } from './flipCard';
import { gameClosed } from './gameClosed';
import { gameHostChanged } from './gameHostChanged';
import { gameSay } from './gameSay';
import { gameStateChanged } from './gameStateChanged';
import { joinGame } from './joinGame';
import { kicked } from './kicked';
import { leaveGame } from './leaveGame';
import { moveCard } from './moveCard';
import { playerPropertiesChanged } from './playerPropertiesChanged';
import { revealCards } from './revealCards';
import { reverseTurn } from './reverseTurn';
import { rollDie } from './rollDie';
import { setActivePhase } from './setActivePhase';
import { setActivePlayer } from './setActivePlayer';
import { setCardAttr } from './setCardAttr';
import { setCardCounter } from './setCardCounter';
import { setCounter } from './setCounter';
import { shuffle } from './shuffle';

useWebClientCleanup();

const meta = { gameId: 5, playerId: 2, context: null, secondsElapsed: 0, forcedByJudge: 0 };

describe('joinGame event', () => {
  it('delegates to WebClient.instance.response.game.playerJoined with gameId from meta', () => {
    const playerProperties = create(ServerInfo_PlayerPropertiesSchema, { playerId: 1 });
    const data = { playerProperties };
    joinGame(data, meta);
    expect(WebClient.instance.response.game.playerJoined).toHaveBeenCalledWith(5, playerProperties);
  });
});

describe('leaveGame event', () => {
  it('delegates to WebClient.instance.response.game.playerLeft with gameId/playerId from meta', () => {
    const data = { reason: 3 };
    leaveGame(data, meta);
    expect(WebClient.instance.response.game.playerLeft).toHaveBeenCalledWith(5, 2, 3);
  });
});

describe('gameClosed event', () => {
  it('delegates to WebClient.instance.response.game.gameClosed with gameId', () => {
    gameClosed({}, meta);
    expect(WebClient.instance.response.game.gameClosed).toHaveBeenCalledWith(5);
  });
});

describe('gameHostChanged event', () => {
  it('delegates to WebClient.instance.response.game.gameHostChanged using meta.playerId as hostId', () => {
    gameHostChanged({}, meta);
    expect(WebClient.instance.response.game.gameHostChanged).toHaveBeenCalledWith(5, 2);
  });
});

describe('kicked event', () => {
  it('delegates to WebClient.instance.response.game.kicked with gameId', () => {
    kicked({}, meta);
    expect(WebClient.instance.response.game.kicked).toHaveBeenCalledWith(5);
  });
});

describe('gameStateChanged event', () => {
  it('delegates to WebClient.instance.response.game.gameStateChanged with gameId and full data', () => {
    const data = create(Event_GameStateChangedSchema, { playerList: [] });
    gameStateChanged(data, meta);
    expect(WebClient.instance.response.game.gameStateChanged).toHaveBeenCalledWith(5, data);
  });
});

describe('playerPropertiesChanged event', () => {
  it('delegates to WebClient.instance.response.game.playerPropertiesChanged with gameId, playerId, properties', () => {
    const playerProperties = create(ServerInfo_PlayerPropertiesSchema, { playerId: 2 });
    const data = { playerProperties };
    playerPropertiesChanged(data, meta);
    expect(WebClient.instance.response.game.playerPropertiesChanged).toHaveBeenCalledWith(5, 2, playerProperties);
  });
});

describe('gameSay event', () => {
  it('delegates to WebClient.instance.response.game.gameSay with gameId, playerId, message', () => {
    const data = create(Event_GameSaySchema, { message: 'gg' });
    gameSay(data, meta);
    expect(WebClient.instance.response.game.gameSay).toHaveBeenCalledWith(5, 2, 'gg');
  });
});

describe('moveCard event', () => {
  it('delegates to WebClient.instance.response.game.cardMoved with gameId, playerId and data', () => {
    const data = create(Event_MoveCardSchema, { cardId: 3 });
    moveCard(data, meta);
    expect(WebClient.instance.response.game.cardMoved).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('flipCard event', () => {
  it('delegates to WebClient.instance.response.game.cardFlipped with gameId, playerId and data', () => {
    const data = create(Event_FlipCardSchema, { cardId: 3 });
    flipCard(data, meta);
    expect(WebClient.instance.response.game.cardFlipped).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('destroyCard event', () => {
  it('delegates to WebClient.instance.response.game.cardDestroyed with gameId, playerId and data', () => {
    const data = create(Event_DestroyCardSchema, { cardId: 3 });
    destroyCard(data, meta);
    expect(WebClient.instance.response.game.cardDestroyed).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('attachCard event', () => {
  it('delegates to WebClient.instance.response.game.cardAttached with gameId, playerId and data', () => {
    const data = create(Event_AttachCardSchema, { cardId: 3 });
    attachCard(data, meta);
    expect(WebClient.instance.response.game.cardAttached).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createToken event', () => {
  it('delegates to WebClient.instance.response.game.tokenCreated with gameId, playerId and data', () => {
    const data = create(Event_CreateTokenSchema, { cardId: 3 });
    createToken(data, meta);
    expect(WebClient.instance.response.game.tokenCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCardAttr event', () => {
  it('delegates to WebClient.instance.response.game.cardAttrChanged with gameId, playerId and data', () => {
    const data = create(Event_SetCardAttrSchema, { cardId: 3 });
    setCardAttr(data, meta);
    expect(WebClient.instance.response.game.cardAttrChanged).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCardCounter event', () => {
  it('delegates to WebClient.instance.response.game.cardCounterChanged with gameId, playerId and data', () => {
    const data = create(Event_SetCardCounterSchema, { cardId: 3 });
    setCardCounter(data, meta);
    expect(WebClient.instance.response.game.cardCounterChanged).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createArrow event', () => {
  it('delegates to WebClient.instance.response.game.arrowCreated with gameId, playerId and data', () => {
    const data = create(Event_CreateArrowSchema, {});
    createArrow(data, meta);
    expect(WebClient.instance.response.game.arrowCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('deleteArrow event', () => {
  it('delegates to WebClient.instance.response.game.arrowDeleted with gameId, playerId and data', () => {
    const data = create(Event_DeleteArrowSchema, { arrowId: 9 });
    deleteArrow(data, meta);
    expect(WebClient.instance.response.game.arrowDeleted).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createCounter event', () => {
  it('delegates to WebClient.instance.response.game.counterCreated with gameId, playerId and data', () => {
    const data = create(Event_CreateCounterSchema, {});
    createCounter(data, meta);
    expect(WebClient.instance.response.game.counterCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCounter event', () => {
  it('delegates to WebClient.instance.response.game.counterSet with gameId, playerId and data', () => {
    const data = create(Event_SetCounterSchema, { counterId: 1, value: 20 });
    setCounter(data, meta);
    expect(WebClient.instance.response.game.counterSet).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('delCounter event', () => {
  it('delegates to WebClient.instance.response.game.counterDeleted with gameId, playerId and data', () => {
    const data = create(Event_DelCounterSchema, { counterId: 1 });
    delCounter(data, meta);
    expect(WebClient.instance.response.game.counterDeleted).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('drawCards event', () => {
  it('delegates to WebClient.instance.response.game.cardsDrawn with gameId, playerId and data', () => {
    const data = create(Event_DrawCardsSchema, { number: 2, cards: [] });
    drawCards(data, meta);
    expect(WebClient.instance.response.game.cardsDrawn).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('revealCards event', () => {
  it('delegates to WebClient.instance.response.game.cardsRevealed with gameId, playerId and data', () => {
    const data = create(Event_RevealCardsSchema, { zoneName: 'hand', cards: [] });
    revealCards(data, meta);
    expect(WebClient.instance.response.game.cardsRevealed).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('shuffle event', () => {
  it('delegates to WebClient.instance.response.game.zoneShuffled with gameId, playerId and data', () => {
    const data = create(Event_ShuffleSchema, { zoneName: 'deck' });
    shuffle(data, meta);
    expect(WebClient.instance.response.game.zoneShuffled).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('rollDie event', () => {
  it('delegates to WebClient.instance.response.game.dieRolled with gameId, playerId and data', () => {
    const data = create(Event_RollDieSchema, { die: 6, result: 4 });
    rollDie(data, meta);
    expect(WebClient.instance.response.game.dieRolled).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setActivePlayer event', () => {
  it('delegates to WebClient.instance.response.game.activePlayerSet with gameId and activePlayerId', () => {
    const data = create(Event_SetActivePlayerSchema, { activePlayerId: 3 });
    setActivePlayer(data, meta);
    expect(WebClient.instance.response.game.activePlayerSet).toHaveBeenCalledWith(5, 3);
  });
});

describe('setActivePhase event', () => {
  it('delegates to WebClient.instance.response.game.activePhaseSet with gameId and phase', () => {
    const data = create(Event_SetActivePhaseSchema, { phase: 4 });
    setActivePhase(data, meta);
    expect(WebClient.instance.response.game.activePhaseSet).toHaveBeenCalledWith(5, 4);
  });
});

describe('reverseTurn event', () => {
  it('delegates to WebClient.instance.response.game.turnReversed with gameId and reversed', () => {
    const data = create(Event_ReverseTurnSchema, { reversed: true });
    reverseTurn(data, meta);
    expect(WebClient.instance.response.game.turnReversed).toHaveBeenCalledWith(5, true);
  });
});

describe('dumpZone event', () => {
  it('delegates to WebClient.instance.response.game.zoneDumped with gameId, playerId and data', () => {
    const data = create(Event_DumpZoneSchema, { zoneName: 'hand' });
    dumpZone(data, meta);
    expect(WebClient.instance.response.game.zoneDumped).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('changeZoneProperties event', () => {
  it('delegates to WebClient.instance.response.game.zonePropertiesChanged with gameId, playerId and data', () => {
    const data = create(Event_ChangeZonePropertiesSchema, { zoneName: 'hand', alwaysRevealTopCard: true });
    changeZoneProperties(data, meta);
    expect(WebClient.instance.response.game.zonePropertiesChanged).toHaveBeenCalledWith(5, 2, data);
  });
});
