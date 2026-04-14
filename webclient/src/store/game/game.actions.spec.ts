import { create } from '@bufbuild/protobuf';
import { Actions } from './game.actions';
import { Types } from './game.types';
import {
  makeArrow,
  makeCard,
  makeCounter,
  makeGameEntry,
  makePlayerProperties,
} from './__mocks__/fixtures';
import { Event_GameStateChangedSchema } from 'generated/proto/event_game_state_changed_pb';
import { Event_MoveCardSchema } from 'generated/proto/event_move_card_pb';
import { Event_FlipCardSchema } from 'generated/proto/event_flip_card_pb';
import { Event_DestroyCardSchema } from 'generated/proto/event_destroy_card_pb';
import { Event_AttachCardSchema } from 'generated/proto/event_attach_card_pb';
import { Event_CreateTokenSchema } from 'generated/proto/event_create_token_pb';
import { Event_SetCardAttrSchema } from 'generated/proto/event_set_card_attr_pb';
import { Event_SetCardCounterSchema } from 'generated/proto/event_set_card_counter_pb';
import { Event_CreateArrowSchema } from 'generated/proto/event_create_arrow_pb';
import { Event_DeleteArrowSchema } from 'generated/proto/event_delete_arrow_pb';
import { Event_CreateCounterSchema } from 'generated/proto/event_create_counter_pb';
import { Event_SetCounterSchema } from 'generated/proto/event_set_counter_pb';
import { Event_DelCounterSchema } from 'generated/proto/event_del_counter_pb';
import { Event_DrawCardsSchema } from 'generated/proto/event_draw_cards_pb';
import { Event_RevealCardsSchema } from 'generated/proto/event_reveal_cards_pb';
import { Event_ShuffleSchema } from 'generated/proto/event_shuffle_pb';
import { Event_RollDieSchema } from 'generated/proto/event_roll_die_pb';
import { Event_DumpZoneSchema } from 'generated/proto/event_dump_zone_pb';
import { Event_ChangeZonePropertiesSchema } from 'generated/proto/event_change_zone_properties_pb';

describe('Actions', () => {
  it('clearStore', () => {
    expect(Actions.clearStore()).toEqual({ type: Types.CLEAR_STORE });
  });

  it('gameJoined', () => {
    const entry = makeGameEntry();
    expect(Actions.gameJoined(1, entry)).toEqual({ type: Types.GAME_JOINED, gameId: 1, gameEntry: entry });
  });

  it('gameLeft', () => {
    expect(Actions.gameLeft(2)).toEqual({ type: Types.GAME_LEFT, gameId: 2 });
  });

  it('gameClosed', () => {
    expect(Actions.gameClosed(3)).toEqual({ type: Types.GAME_CLOSED, gameId: 3 });
  });

  it('gameHostChanged', () => {
    expect(Actions.gameHostChanged(1, 7)).toEqual({ type: Types.GAME_HOST_CHANGED, gameId: 1, hostId: 7 });
  });

  it('gameStateChanged', () => {
    const data = create(Event_GameStateChangedSchema, {
      playerList: [], gameStarted: true, activePlayerId: 1, activePhase: 0, secondsElapsed: 0
    });
    expect(Actions.gameStateChanged(1, data)).toEqual({ type: Types.GAME_STATE_CHANGED, gameId: 1, data });
  });

  it('playerJoined', () => {
    const props = makePlayerProperties();
    expect(Actions.playerJoined(1, props)).toEqual({ type: Types.PLAYER_JOINED, gameId: 1, playerProperties: props });
  });

  it('playerLeft', () => {
    expect(Actions.playerLeft(1, 2, 3)).toEqual({ type: Types.PLAYER_LEFT, gameId: 1, playerId: 2, reason: 3 });
  });

  it('playerPropertiesChanged', () => {
    const props = makePlayerProperties();
    expect(Actions.playerPropertiesChanged(1, 2, props)).toEqual({
      type: Types.PLAYER_PROPERTIES_CHANGED,
      gameId: 1,
      playerId: 2,
      properties: props,
    });
  });

  it('kicked', () => {
    expect(Actions.kicked(1)).toEqual({ type: Types.KICKED, gameId: 1 });
  });

  it('cardMoved', () => {
    const data = create(Event_MoveCardSchema, { cardId: 1 });
    expect(Actions.cardMoved(1, 2, data)).toEqual({ type: Types.CARD_MOVED, gameId: 1, playerId: 2, data });
  });

  it('cardFlipped', () => {
    const data = create(Event_FlipCardSchema, { cardId: 1 });
    expect(Actions.cardFlipped(1, 2, data)).toEqual({ type: Types.CARD_FLIPPED, gameId: 1, playerId: 2, data });
  });

  it('cardDestroyed', () => {
    const data = create(Event_DestroyCardSchema, { cardId: 1 });
    expect(Actions.cardDestroyed(1, 2, data)).toEqual({ type: Types.CARD_DESTROYED, gameId: 1, playerId: 2, data });
  });

  it('cardAttached', () => {
    const data = create(Event_AttachCardSchema, { cardId: 1 });
    expect(Actions.cardAttached(1, 2, data)).toEqual({ type: Types.CARD_ATTACHED, gameId: 1, playerId: 2, data });
  });

  it('tokenCreated', () => {
    const data = create(Event_CreateTokenSchema, { cardId: 1 });
    expect(Actions.tokenCreated(1, 2, data)).toEqual({ type: Types.TOKEN_CREATED, gameId: 1, playerId: 2, data });
  });

  it('cardAttrChanged', () => {
    const data = create(Event_SetCardAttrSchema, { cardId: 1 });
    expect(Actions.cardAttrChanged(1, 2, data)).toEqual({ type: Types.CARD_ATTR_CHANGED, gameId: 1, playerId: 2, data });
  });

  it('cardCounterChanged', () => {
    const data = create(Event_SetCardCounterSchema, { cardId: 1 });
    expect(Actions.cardCounterChanged(1, 2, data)).toEqual({ type: Types.CARD_COUNTER_CHANGED, gameId: 1, playerId: 2, data });
  });

  it('arrowCreated', () => {
    const arrow = makeArrow();
    const data = create(Event_CreateArrowSchema, { arrowInfo: arrow });
    expect(Actions.arrowCreated(1, 2, data)).toEqual({ type: Types.ARROW_CREATED, gameId: 1, playerId: 2, data });
  });

  it('arrowDeleted', () => {
    const data = create(Event_DeleteArrowSchema, { arrowId: 3 });
    expect(Actions.arrowDeleted(1, 2, data)).toEqual({ type: Types.ARROW_DELETED, gameId: 1, playerId: 2, data });
  });

  it('counterCreated', () => {
    const counter = makeCounter();
    const data = create(Event_CreateCounterSchema, { counterInfo: counter });
    expect(Actions.counterCreated(1, 2, data)).toEqual({ type: Types.COUNTER_CREATED, gameId: 1, playerId: 2, data });
  });

  it('counterSet', () => {
    const data = create(Event_SetCounterSchema, { counterId: 1, value: 10 });
    expect(Actions.counterSet(1, 2, data)).toEqual({ type: Types.COUNTER_SET, gameId: 1, playerId: 2, data });
  });

  it('counterDeleted', () => {
    const data = create(Event_DelCounterSchema, { counterId: 1 });
    expect(Actions.counterDeleted(1, 2, data)).toEqual({ type: Types.COUNTER_DELETED, gameId: 1, playerId: 2, data });
  });

  it('cardsDrawn', () => {
    const card = makeCard();
    const data = create(Event_DrawCardsSchema, { number: 2, cards: [card] });
    expect(Actions.cardsDrawn(1, 2, data)).toEqual({ type: Types.CARDS_DRAWN, gameId: 1, playerId: 2, data });
  });

  it('cardsRevealed', () => {
    const data = create(Event_RevealCardsSchema, { zoneName: 'hand', cards: [] });
    expect(Actions.cardsRevealed(1, 2, data)).toEqual({ type: Types.CARDS_REVEALED, gameId: 1, playerId: 2, data });
  });

  it('zoneShuffled', () => {
    const data = create(Event_ShuffleSchema, { zoneName: 'deck', start: 0, end: 39 });
    expect(Actions.zoneShuffled(1, 2, data)).toEqual({ type: Types.ZONE_SHUFFLED, gameId: 1, playerId: 2, data });
  });

  it('dieRolled', () => {
    const data = create(Event_RollDieSchema, { sides: 6, value: 4, values: [4] });
    expect(Actions.dieRolled(1, 2, data)).toEqual({ type: Types.DIE_ROLLED, gameId: 1, playerId: 2, data });
  });

  it('activePlayerSet', () => {
    expect(Actions.activePlayerSet(1, 3)).toEqual({ type: Types.ACTIVE_PLAYER_SET, gameId: 1, activePlayerId: 3 });
  });

  it('activePhaseSet', () => {
    expect(Actions.activePhaseSet(1, 2)).toEqual({ type: Types.ACTIVE_PHASE_SET, gameId: 1, phase: 2 });
  });

  it('turnReversed', () => {
    expect(Actions.turnReversed(1, true)).toEqual({ type: Types.TURN_REVERSED, gameId: 1, reversed: true });
  });

  it('zoneDumped', () => {
    const data = create(Event_DumpZoneSchema, { zoneOwnerId: 1, zoneName: 'hand', numberCards: 3, isReversed: false });
    expect(Actions.zoneDumped(1, 2, data)).toEqual({ type: Types.ZONE_DUMPED, gameId: 1, playerId: 2, data });
  });

  it('zonePropertiesChanged', () => {
    const data = create(Event_ChangeZonePropertiesSchema, { zoneName: 'deck', alwaysRevealTopCard: true, alwaysLookAtTopCard: false });
    expect(Actions.zonePropertiesChanged(1, 2, data)).toEqual({
      type: Types.ZONE_PROPERTIES_CHANGED,
      gameId: 1,
      playerId: 2,
      data,
    });
  });

  it('gameSay', () => {
    expect(Actions.gameSay(1, 2, 'hello')).toEqual({ type: Types.GAME_SAY, gameId: 1, playerId: 2, message: 'hello' });
  });
});
