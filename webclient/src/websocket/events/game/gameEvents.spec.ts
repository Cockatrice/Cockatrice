jest.mock('../../persistence', () => ({
  GamePersistence: {
    gameStateChanged: jest.fn(),
    playerJoined: jest.fn(),
    playerLeft: jest.fn(),
    playerPropertiesChanged: jest.fn(),
    gameClosed: jest.fn(),
    gameHostChanged: jest.fn(),
    kicked: jest.fn(),
    gameSay: jest.fn(),
    cardMoved: jest.fn(),
    cardFlipped: jest.fn(),
    cardDestroyed: jest.fn(),
    cardAttached: jest.fn(),
    tokenCreated: jest.fn(),
    cardAttrChanged: jest.fn(),
    cardCounterChanged: jest.fn(),
    arrowCreated: jest.fn(),
    arrowDeleted: jest.fn(),
    counterCreated: jest.fn(),
    counterSet: jest.fn(),
    counterDeleted: jest.fn(),
    cardsDrawn: jest.fn(),
    cardsRevealed: jest.fn(),
    zoneShuffled: jest.fn(),
    dieRolled: jest.fn(),
    activePlayerSet: jest.fn(),
    activePhaseSet: jest.fn(),
    turnReversed: jest.fn(),
    zoneDumped: jest.fn(),
    zonePropertiesChanged: jest.fn(),
  },
}));

import { GamePersistence } from '../../persistence';
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

beforeEach(() => jest.clearAllMocks());

const meta = { gameId: 5, playerId: 2, context: null, secondsElapsed: 0, forcedByJudge: 0 };

describe('joinGame event', () => {
  it('delegates to GamePersistence.playerJoined with gameId from meta', () => {
    const playerProperties = { playerId: 1 };
    const data = { playerProperties } as any;
    joinGame(data, meta);
    expect(GamePersistence.playerJoined).toHaveBeenCalledWith(5, playerProperties);
  });
});

describe('leaveGame event', () => {
  it('delegates to GamePersistence.playerLeft with gameId/playerId from meta', () => {
    const data = { reason: 3 };
    leaveGame(data, meta);
    expect(GamePersistence.playerLeft).toHaveBeenCalledWith(5, 2, 3);
  });
});

describe('gameClosed event', () => {
  it('delegates to GamePersistence.gameClosed with gameId', () => {
    gameClosed({}, meta);
    expect(GamePersistence.gameClosed).toHaveBeenCalledWith(5);
  });
});

describe('gameHostChanged event', () => {
  it('delegates to GamePersistence.gameHostChanged using meta.playerId as hostId', () => {
    gameHostChanged({}, meta);
    expect(GamePersistence.gameHostChanged).toHaveBeenCalledWith(5, 2);
  });
});

describe('kicked event', () => {
  it('delegates to GamePersistence.kicked with gameId', () => {
    kicked({}, meta);
    expect(GamePersistence.kicked).toHaveBeenCalledWith(5);
  });
});

describe('gameStateChanged event', () => {
  it('delegates to GamePersistence.gameStateChanged with gameId and full data', () => {
    const data = { playerList: [] } as any;
    gameStateChanged(data, meta);
    expect(GamePersistence.gameStateChanged).toHaveBeenCalledWith(5, data);
  });
});

describe('playerPropertiesChanged event', () => {
  it('delegates to GamePersistence.playerPropertiesChanged with gameId, playerId, properties', () => {
    const playerProperties = { playerId: 2 } as any;
    const data = { playerProperties } as any;
    playerPropertiesChanged(data, meta);
    expect(GamePersistence.playerPropertiesChanged).toHaveBeenCalledWith(5, 2, playerProperties);
  });
});

describe('gameSay event', () => {
  it('delegates to GamePersistence.gameSay with gameId, playerId, message', () => {
    const data = { message: 'gg' } as any;
    gameSay(data, meta);
    expect(GamePersistence.gameSay).toHaveBeenCalledWith(5, 2, 'gg');
  });
});

describe('moveCard event', () => {
  it('delegates to GamePersistence.cardMoved with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    moveCard(data, meta);
    expect(GamePersistence.cardMoved).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('flipCard event', () => {
  it('delegates to GamePersistence.cardFlipped with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    flipCard(data, meta);
    expect(GamePersistence.cardFlipped).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('destroyCard event', () => {
  it('delegates to GamePersistence.cardDestroyed with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    destroyCard(data, meta);
    expect(GamePersistence.cardDestroyed).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('attachCard event', () => {
  it('delegates to GamePersistence.cardAttached with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    attachCard(data, meta);
    expect(GamePersistence.cardAttached).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createToken event', () => {
  it('delegates to GamePersistence.tokenCreated with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    createToken(data, meta);
    expect(GamePersistence.tokenCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCardAttr event', () => {
  it('delegates to GamePersistence.cardAttrChanged with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    setCardAttr(data, meta);
    expect(GamePersistence.cardAttrChanged).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCardCounter event', () => {
  it('delegates to GamePersistence.cardCounterChanged with gameId, playerId and data', () => {
    const data = { cardId: 3 } as any;
    setCardCounter(data, meta);
    expect(GamePersistence.cardCounterChanged).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createArrow event', () => {
  it('delegates to GamePersistence.arrowCreated with gameId, playerId and data', () => {
    const data = { arrowInfo: {} } as any;
    createArrow(data, meta);
    expect(GamePersistence.arrowCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('deleteArrow event', () => {
  it('delegates to GamePersistence.arrowDeleted with gameId, playerId and data', () => {
    const data = { arrowId: 9 } as any;
    deleteArrow(data, meta);
    expect(GamePersistence.arrowDeleted).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('createCounter event', () => {
  it('delegates to GamePersistence.counterCreated with gameId, playerId and data', () => {
    const data = { counterInfo: {} } as any;
    createCounter(data, meta);
    expect(GamePersistence.counterCreated).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setCounter event', () => {
  it('delegates to GamePersistence.counterSet with gameId, playerId and data', () => {
    const data = { counterId: 1, value: 20 } as any;
    setCounter(data, meta);
    expect(GamePersistence.counterSet).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('delCounter event', () => {
  it('delegates to GamePersistence.counterDeleted with gameId, playerId and data', () => {
    const data = { counterId: 1 } as any;
    delCounter(data, meta);
    expect(GamePersistence.counterDeleted).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('drawCards event', () => {
  it('delegates to GamePersistence.cardsDrawn with gameId, playerId and data', () => {
    const data = { number: 2, cards: [] } as any;
    drawCards(data, meta);
    expect(GamePersistence.cardsDrawn).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('revealCards event', () => {
  it('delegates to GamePersistence.cardsRevealed with gameId, playerId and data', () => {
    const data = { zoneName: 'hand', cards: [] } as any;
    revealCards(data, meta);
    expect(GamePersistence.cardsRevealed).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('shuffle event', () => {
  it('delegates to GamePersistence.zoneShuffled with gameId, playerId and data', () => {
    const data = { zoneName: 'deck' } as any;
    shuffle(data, meta);
    expect(GamePersistence.zoneShuffled).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('rollDie event', () => {
  it('delegates to GamePersistence.dieRolled with gameId, playerId and data', () => {
    const data = { die: 6, result: 4 } as any;
    rollDie(data, meta);
    expect(GamePersistence.dieRolled).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('setActivePlayer event', () => {
  it('delegates to GamePersistence.activePlayerSet with gameId and activePlayerId', () => {
    const data = { activePlayerId: 3 } as any;
    setActivePlayer(data, meta);
    expect(GamePersistence.activePlayerSet).toHaveBeenCalledWith(5, 3);
  });
});

describe('setActivePhase event', () => {
  it('delegates to GamePersistence.activePhaseSet with gameId and phase', () => {
    const data = { phase: 4 } as any;
    setActivePhase(data, meta);
    expect(GamePersistence.activePhaseSet).toHaveBeenCalledWith(5, 4);
  });
});

describe('reverseTurn event', () => {
  it('delegates to GamePersistence.turnReversed with gameId and reversed', () => {
    const data = { reversed: true } as any;
    reverseTurn(data, meta);
    expect(GamePersistence.turnReversed).toHaveBeenCalledWith(5, true);
  });
});

describe('dumpZone event', () => {
  it('delegates to GamePersistence.zoneDumped with gameId, playerId and data', () => {
    const data = { zoneName: 'hand' } as any;
    dumpZone(data, meta);
    expect(GamePersistence.zoneDumped).toHaveBeenCalledWith(5, 2, data);
  });
});

describe('changeZoneProperties event', () => {
  it('delegates to GamePersistence.zonePropertiesChanged with gameId, playerId and data', () => {
    const data = { zoneName: 'hand', alwaysRevealTopCard: true } as any;
    changeZoneProperties(data, meta);
    expect(GamePersistence.zonePropertiesChanged).toHaveBeenCalledWith(5, 2, data);
  });
});
