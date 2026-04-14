import { GameDispatch } from 'store';
import type { Event_AttachCard } from 'generated/proto/event_attach_card_pb';
import type { Event_ChangeZoneProperties } from 'generated/proto/event_change_zone_properties_pb';
import type { Event_CreateArrow } from 'generated/proto/event_create_arrow_pb';
import type { Event_CreateCounter } from 'generated/proto/event_create_counter_pb';
import type { Event_CreateToken } from 'generated/proto/event_create_token_pb';
import type { Event_DelCounter } from 'generated/proto/event_del_counter_pb';
import type { Event_DeleteArrow } from 'generated/proto/event_delete_arrow_pb';
import type { Event_DestroyCard } from 'generated/proto/event_destroy_card_pb';
import type { Event_DrawCards } from 'generated/proto/event_draw_cards_pb';
import type { Event_DumpZone } from 'generated/proto/event_dump_zone_pb';
import type { Event_FlipCard } from 'generated/proto/event_flip_card_pb';
import type { Event_GameStateChanged } from 'generated/proto/event_game_state_changed_pb';
import type { Event_MoveCard } from 'generated/proto/event_move_card_pb';
import type { ServerInfo_PlayerProperties } from 'generated/proto/serverinfo_playerproperties_pb';
import type { Event_RevealCards } from 'generated/proto/event_reveal_cards_pb';
import type { Event_RollDie } from 'generated/proto/event_roll_die_pb';
import type { Event_SetCardAttr } from 'generated/proto/event_set_card_attr_pb';
import type { Event_SetCardCounter } from 'generated/proto/event_set_card_counter_pb';
import type { Event_SetCounter } from 'generated/proto/event_set_counter_pb';
import type { Event_Shuffle } from 'generated/proto/event_shuffle_pb';

export class GamePersistence {
  static gameStateChanged(gameId: number, data: Event_GameStateChanged): void {
    GameDispatch.gameStateChanged(gameId, data);
  }

  static playerJoined(gameId: number, playerProperties: ServerInfo_PlayerProperties): void {
    GameDispatch.playerJoined(gameId, playerProperties);
  }

  static playerLeft(gameId: number, playerId: number, reason: number): void {
    GameDispatch.playerLeft(gameId, playerId, reason);
  }

  static playerPropertiesChanged(gameId: number, playerId: number, properties: ServerInfo_PlayerProperties): void {
    GameDispatch.playerPropertiesChanged(gameId, playerId, properties);
  }

  static gameClosed(gameId: number): void {
    GameDispatch.gameClosed(gameId);
  }

  static gameHostChanged(gameId: number, hostId: number): void {
    GameDispatch.gameHostChanged(gameId, hostId);
  }

  static kicked(gameId: number): void {
    GameDispatch.kicked(gameId);
  }

  static gameSay(gameId: number, playerId: number, message: string): void {
    GameDispatch.gameSay(gameId, playerId, message);
  }

  static cardMoved(gameId: number, playerId: number, data: Event_MoveCard): void {
    GameDispatch.cardMoved(gameId, playerId, data);
  }

  static cardFlipped(gameId: number, playerId: number, data: Event_FlipCard): void {
    GameDispatch.cardFlipped(gameId, playerId, data);
  }

  static cardDestroyed(gameId: number, playerId: number, data: Event_DestroyCard): void {
    GameDispatch.cardDestroyed(gameId, playerId, data);
  }

  static cardAttached(gameId: number, playerId: number, data: Event_AttachCard): void {
    GameDispatch.cardAttached(gameId, playerId, data);
  }

  static tokenCreated(gameId: number, playerId: number, data: Event_CreateToken): void {
    GameDispatch.tokenCreated(gameId, playerId, data);
  }

  static cardAttrChanged(gameId: number, playerId: number, data: Event_SetCardAttr): void {
    GameDispatch.cardAttrChanged(gameId, playerId, data);
  }

  static cardCounterChanged(gameId: number, playerId: number, data: Event_SetCardCounter): void {
    GameDispatch.cardCounterChanged(gameId, playerId, data);
  }

  static arrowCreated(gameId: number, playerId: number, data: Event_CreateArrow): void {
    GameDispatch.arrowCreated(gameId, playerId, data);
  }

  static arrowDeleted(gameId: number, playerId: number, data: Event_DeleteArrow): void {
    GameDispatch.arrowDeleted(gameId, playerId, data);
  }

  static counterCreated(gameId: number, playerId: number, data: Event_CreateCounter): void {
    GameDispatch.counterCreated(gameId, playerId, data);
  }

  static counterSet(gameId: number, playerId: number, data: Event_SetCounter): void {
    GameDispatch.counterSet(gameId, playerId, data);
  }

  static counterDeleted(gameId: number, playerId: number, data: Event_DelCounter): void {
    GameDispatch.counterDeleted(gameId, playerId, data);
  }

  static cardsDrawn(gameId: number, playerId: number, data: Event_DrawCards): void {
    GameDispatch.cardsDrawn(gameId, playerId, data);
  }

  static cardsRevealed(gameId: number, playerId: number, data: Event_RevealCards): void {
    GameDispatch.cardsRevealed(gameId, playerId, data);
  }

  static zoneShuffled(gameId: number, playerId: number, data: Event_Shuffle): void {
    GameDispatch.zoneShuffled(gameId, playerId, data);
  }

  static dieRolled(gameId: number, playerId: number, data: Event_RollDie): void {
    GameDispatch.dieRolled(gameId, playerId, data);
  }

  static activePlayerSet(gameId: number, activePlayerId: number): void {
    GameDispatch.activePlayerSet(gameId, activePlayerId);
  }

  static activePhaseSet(gameId: number, phase: number): void {
    GameDispatch.activePhaseSet(gameId, phase);
  }

  static turnReversed(gameId: number, reversed: boolean): void {
    GameDispatch.turnReversed(gameId, reversed);
  }

  static zoneDumped(gameId: number, playerId: number, data: Event_DumpZone): void {
    GameDispatch.zoneDumped(gameId, playerId, data);
  }

  static zonePropertiesChanged(gameId: number, playerId: number, data: Event_ChangeZoneProperties): void {
    GameDispatch.zonePropertiesChanged(gameId, playerId, data);
  }
}

