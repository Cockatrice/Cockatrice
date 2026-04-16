import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { Actions } from './game.actions';
import {
  makeArrow,
  makeCard,
  makeCounter,
  makePlayerProperties,
} from './__mocks__/fixtures';

describe('Actions', () => {
  it('clearStore', () => {
    const action = Actions.clearStore();
    expect(action.type).toBe('games/clearStore');
  });

  it('gameJoined', () => {
    const data = create(Data.Event_GameJoinedSchema, { hostId: 1, playerId: 2 });
    const action = Actions.gameJoined({ data });
    expect(action.payload.data).toBe(data);
  });

  it('gameLeft', () => {
    const action = Actions.gameLeft({ gameId: 2 });
    expect(action.payload.gameId).toBe(2);
  });

  it('gameClosed', () => {
    const action = Actions.gameClosed({ gameId: 3 });
    expect(action.payload.gameId).toBe(3);
  });

  it('gameHostChanged', () => {
    const action = Actions.gameHostChanged({ gameId: 1, hostId: 7 });
    expect(action.payload).toEqual({ gameId: 1, hostId: 7 });
  });

  it('gameStateChanged', () => {
    const data = create(Data.Event_GameStateChangedSchema, {
      playerList: [], gameStarted: true, activePlayerId: 1, activePhase: 0, secondsElapsed: 0
    });
    const action = Actions.gameStateChanged({ gameId: 1, data });
    expect(action.payload).toEqual({ gameId: 1, data });
  });

  it('playerJoined', () => {
    const props = makePlayerProperties();
    const action = Actions.playerJoined({ gameId: 1, playerProperties: props });
    expect(action.payload.playerProperties).toBe(props);
  });

  it('playerLeft', () => {
    const action = Actions.playerLeft({ gameId: 1, playerId: 2 });
    expect(action.payload).toEqual({ gameId: 1, playerId: 2 });
  });

  it('playerPropertiesChanged', () => {
    const props = makePlayerProperties();
    const action = Actions.playerPropertiesChanged({ gameId: 1, playerId: 2, properties: props });
    expect(action.payload.properties).toBe(props);
  });

  it('kicked', () => {
    const action = Actions.kicked({ gameId: 1 });
    expect(action.payload.gameId).toBe(1);
  });

  it('cardMoved', () => {
    const data = create(Data.Event_MoveCardSchema, { cardId: 1 });
    const action = Actions.cardMoved({ gameId: 1, playerId: 2, data });
    expect(action.payload).toEqual({ gameId: 1, playerId: 2, data });
  });

  it('cardFlipped', () => {
    const data = create(Data.Event_FlipCardSchema, { cardId: 1 });
    const action = Actions.cardFlipped({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardDestroyed', () => {
    const data = create(Data.Event_DestroyCardSchema, { cardId: 1 });
    const action = Actions.cardDestroyed({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardAttached', () => {
    const data = create(Data.Event_AttachCardSchema, { cardId: 1 });
    const action = Actions.cardAttached({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('tokenCreated', () => {
    const data = create(Data.Event_CreateTokenSchema, { cardId: 1 });
    const action = Actions.tokenCreated({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardAttrChanged', () => {
    const data = create(Data.Event_SetCardAttrSchema, { cardId: 1 });
    const action = Actions.cardAttrChanged({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardCounterChanged', () => {
    const data = create(Data.Event_SetCardCounterSchema, { cardId: 1 });
    const action = Actions.cardCounterChanged({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('arrowCreated', () => {
    const arrow = makeArrow();
    const data = create(Data.Event_CreateArrowSchema, { arrowInfo: arrow });
    const action = Actions.arrowCreated({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('arrowDeleted', () => {
    const data = create(Data.Event_DeleteArrowSchema, { arrowId: 3 });
    const action = Actions.arrowDeleted({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('counterCreated', () => {
    const counter = makeCounter();
    const data = create(Data.Event_CreateCounterSchema, { counterInfo: counter });
    const action = Actions.counterCreated({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('counterSet', () => {
    const data = create(Data.Event_SetCounterSchema, { counterId: 1, value: 10 });
    const action = Actions.counterSet({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('counterDeleted', () => {
    const data = create(Data.Event_DelCounterSchema, { counterId: 1 });
    const action = Actions.counterDeleted({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardsDrawn', () => {
    const card = makeCard();
    const data = create(Data.Event_DrawCardsSchema, { number: 2, cards: [card] });
    const action = Actions.cardsDrawn({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('cardsRevealed', () => {
    const data = create(Data.Event_RevealCardsSchema, { zoneName: 'hand', cards: [] });
    const action = Actions.cardsRevealed({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('zoneShuffled', () => {
    const data = create(Data.Event_ShuffleSchema, { zoneName: 'deck', start: 0, end: 39 });
    const action = Actions.zoneShuffled({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('dieRolled', () => {
    const data = create(Data.Event_RollDieSchema, { sides: 6, value: 4, values: [4] });
    const action = Actions.dieRolled({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('activePlayerSet', () => {
    const action = Actions.activePlayerSet({ gameId: 1, activePlayerId: 3 });
    expect(action.payload).toEqual({ gameId: 1, activePlayerId: 3 });
  });

  it('activePhaseSet', () => {
    const action = Actions.activePhaseSet({ gameId: 1, phase: 2 });
    expect(action.payload).toEqual({ gameId: 1, phase: 2 });
  });

  it('turnReversed', () => {
    const action = Actions.turnReversed({ gameId: 1, reversed: true });
    expect(action.payload).toEqual({ gameId: 1, reversed: true });
  });

  it('zoneDumped', () => {
    const data = create(Data.Event_DumpZoneSchema, { zoneOwnerId: 1, zoneName: 'hand', numberCards: 3, isReversed: false });
    const action = Actions.zoneDumped({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('zonePropertiesChanged', () => {
    const data = create(Data.Event_ChangeZonePropertiesSchema, { zoneName: 'deck', alwaysRevealTopCard: true, alwaysLookAtTopCard: false });
    const action = Actions.zonePropertiesChanged({ gameId: 1, playerId: 2, data });
    expect(action.payload.data).toBe(data);
  });

  it('gameSay', () => {
    const action = Actions.gameSay({ gameId: 1, playerId: 2, message: 'hello' });
    expect(action.payload).toEqual({ gameId: 1, playerId: 2, message: 'hello' });
  });
});
