import { create } from '@bufbuild/protobuf';
import { GamePersistence } from './GamePersistence';

vi.mock('store', () => ({
  GameDispatch: {
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
}));

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
import { ServerInfo_PlayerPropertiesSchema } from 'generated/proto/serverinfo_playerproperties_pb';

import { GameDispatch } from 'store';

beforeEach(() => vi.clearAllMocks());

describe('GamePersistence', () => {
  it('gameStateChanged dispatches via GameDispatch', () => {
    const data = create(Event_GameStateChangedSchema, { playerList: [] });
    GamePersistence.gameStateChanged(5, data);
    expect(GameDispatch.gameStateChanged).toHaveBeenCalledWith(5, data);
  });

  it('playerJoined dispatches via GameDispatch', () => {
    const data = create(ServerInfo_PlayerPropertiesSchema, { playerId: 1 });
    GamePersistence.playerJoined(5, data);
    expect(GameDispatch.playerJoined).toHaveBeenCalledWith(5, data);
  });

  it('playerLeft dispatches via GameDispatch', () => {
    GamePersistence.playerLeft(5, 1, 3);
    expect(GameDispatch.playerLeft).toHaveBeenCalledWith(5, 1, 3);
  });

  it('playerPropertiesChanged dispatches via GameDispatch', () => {
    const props = create(ServerInfo_PlayerPropertiesSchema, { playerId: 2 });
    GamePersistence.playerPropertiesChanged(5, 2, props);
    expect(GameDispatch.playerPropertiesChanged).toHaveBeenCalledWith(5, 2, props);
  });

  it('gameClosed dispatches via GameDispatch', () => {
    GamePersistence.gameClosed(5);
    expect(GameDispatch.gameClosed).toHaveBeenCalledWith(5);
  });

  it('gameHostChanged dispatches via GameDispatch', () => {
    GamePersistence.gameHostChanged(5, 7);
    expect(GameDispatch.gameHostChanged).toHaveBeenCalledWith(5, 7);
  });

  it('kicked dispatches via GameDispatch', () => {
    GamePersistence.kicked(5);
    expect(GameDispatch.kicked).toHaveBeenCalledWith(5);
  });

  it('gameSay dispatches via GameDispatch', () => {
    GamePersistence.gameSay(5, 1, 'hello');
    expect(GameDispatch.gameSay).toHaveBeenCalledWith(5, 1, 'hello');
  });

  it('cardMoved dispatches via GameDispatch', () => {
    const data = create(Event_MoveCardSchema, { cardId: 3 });
    GamePersistence.cardMoved(5, 1, data);
    expect(GameDispatch.cardMoved).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardFlipped dispatches via GameDispatch', () => {
    const data = create(Event_FlipCardSchema, { cardId: 3 });
    GamePersistence.cardFlipped(5, 1, data);
    expect(GameDispatch.cardFlipped).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardDestroyed dispatches via GameDispatch', () => {
    const data = create(Event_DestroyCardSchema, { cardId: 3 });
    GamePersistence.cardDestroyed(5, 1, data);
    expect(GameDispatch.cardDestroyed).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardAttached dispatches via GameDispatch', () => {
    const data = create(Event_AttachCardSchema, { cardId: 3 });
    GamePersistence.cardAttached(5, 1, data);
    expect(GameDispatch.cardAttached).toHaveBeenCalledWith(5, 1, data);
  });

  it('tokenCreated dispatches via GameDispatch', () => {
    const data = create(Event_CreateTokenSchema, { cardId: 3 });
    GamePersistence.tokenCreated(5, 1, data);
    expect(GameDispatch.tokenCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardAttrChanged dispatches via GameDispatch', () => {
    const data = create(Event_SetCardAttrSchema, { cardId: 3 });
    GamePersistence.cardAttrChanged(5, 1, data);
    expect(GameDispatch.cardAttrChanged).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardCounterChanged dispatches via GameDispatch', () => {
    const data = create(Event_SetCardCounterSchema, { cardId: 3 });
    GamePersistence.cardCounterChanged(5, 1, data);
    expect(GameDispatch.cardCounterChanged).toHaveBeenCalledWith(5, 1, data);
  });

  it('arrowCreated dispatches via GameDispatch', () => {
    const data = create(Event_CreateArrowSchema, {});
    GamePersistence.arrowCreated(5, 1, data);
    expect(GameDispatch.arrowCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('arrowDeleted dispatches via GameDispatch', () => {
    const data = create(Event_DeleteArrowSchema, { arrowId: 9 });
    GamePersistence.arrowDeleted(5, 1, data);
    expect(GameDispatch.arrowDeleted).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterCreated dispatches via GameDispatch', () => {
    const data = create(Event_CreateCounterSchema, {});
    GamePersistence.counterCreated(5, 1, data);
    expect(GameDispatch.counterCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterSet dispatches via GameDispatch', () => {
    const data = create(Event_SetCounterSchema, { counterId: 1, value: 20 });
    GamePersistence.counterSet(5, 1, data);
    expect(GameDispatch.counterSet).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterDeleted dispatches via GameDispatch', () => {
    const data = create(Event_DelCounterSchema, { counterId: 1 });
    GamePersistence.counterDeleted(5, 1, data);
    expect(GameDispatch.counterDeleted).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardsDrawn dispatches via GameDispatch', () => {
    const data = create(Event_DrawCardsSchema, { number: 2, cards: [] });
    GamePersistence.cardsDrawn(5, 1, data);
    expect(GameDispatch.cardsDrawn).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardsRevealed dispatches via GameDispatch', () => {
    const data = create(Event_RevealCardsSchema, { zoneName: 'hand', cards: [] });
    GamePersistence.cardsRevealed(5, 1, data);
    expect(GameDispatch.cardsRevealed).toHaveBeenCalledWith(5, 1, data);
  });

  it('zoneShuffled dispatches via GameDispatch', () => {
    const data = create(Event_ShuffleSchema, { zoneName: 'deck' });
    GamePersistence.zoneShuffled(5, 1, data);
    expect(GameDispatch.zoneShuffled).toHaveBeenCalledWith(5, 1, data);
  });

  it('dieRolled dispatches via GameDispatch', () => {
    const data = create(Event_RollDieSchema, { sides: 6, value: 4 });
    GamePersistence.dieRolled(5, 1, data);
    expect(GameDispatch.dieRolled).toHaveBeenCalledWith(5, 1, data);
  });

  it('activePlayerSet dispatches via GameDispatch', () => {
    GamePersistence.activePlayerSet(5, 2);
    expect(GameDispatch.activePlayerSet).toHaveBeenCalledWith(5, 2);
  });

  it('activePhaseSet dispatches via GameDispatch', () => {
    GamePersistence.activePhaseSet(5, 3);
    expect(GameDispatch.activePhaseSet).toHaveBeenCalledWith(5, 3);
  });

  it('turnReversed dispatches via GameDispatch', () => {
    GamePersistence.turnReversed(5, true);
    expect(GameDispatch.turnReversed).toHaveBeenCalledWith(5, true);
  });

  it('zoneDumped dispatches via GameDispatch', () => {
    const data = create(Event_DumpZoneSchema, { zoneName: 'hand' });
    GamePersistence.zoneDumped(5, 1, data);
    expect(GameDispatch.zoneDumped).toHaveBeenCalledWith(5, 1, data);
  });

  it('zonePropertiesChanged dispatches via GameDispatch', () => {
    const data = create(Event_ChangeZonePropertiesSchema, { zoneName: 'hand', alwaysRevealTopCard: true });
    GamePersistence.zonePropertiesChanged(5, 1, data);
    expect(GameDispatch.zonePropertiesChanged).toHaveBeenCalledWith(5, 1, data);
  });
});
