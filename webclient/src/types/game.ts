// ── Imports from generated proto files ───────────────────────────────────────

import type { GameEventContext } from 'generated/proto/game_event_context_pb';
import type { CardToMove, Command_MoveCard } from 'generated/proto/command_move_card_pb';
import type { Command_DrawCards } from 'generated/proto/command_draw_cards_pb';
import type { Command_RollDie } from 'generated/proto/command_roll_die_pb';
import type { Command_Shuffle } from 'generated/proto/command_shuffle_pb';
import type { Command_FlipCard } from 'generated/proto/command_flip_card_pb';
import type { Command_AttachCard } from 'generated/proto/command_attach_card_pb';
import type { Command_CreateToken } from 'generated/proto/command_create_token_pb';
import type { Command_SetCardAttr } from 'generated/proto/command_set_card_attr_pb';
import type { Command_SetCardCounter } from 'generated/proto/command_set_card_counter_pb';
import type { Command_IncCardCounter } from 'generated/proto/command_inc_card_counter_pb';
import type { Command_RevealCards } from 'generated/proto/command_reveal_cards_pb';
import type { Command_DumpZone } from 'generated/proto/command_dump_zone_pb';
import type { Command_ChangeZoneProperties } from 'generated/proto/command_change_zone_properties_pb';
import type { Command_CreateArrow } from 'generated/proto/command_create_arrow_pb';
import type { Command_DeleteArrow } from 'generated/proto/command_delete_arrow_pb';
import type { Command_CreateCounter } from 'generated/proto/command_create_counter_pb';
import type { Command_SetCounter } from 'generated/proto/command_set_counter_pb';
import type { Command_IncCounter } from 'generated/proto/command_inc_counter_pb';
import type { Command_DelCounter } from 'generated/proto/command_del_counter_pb';
import type { Command_KickFromGame } from 'generated/proto/command_kick_from_game_pb';
import type { Command_ReadyStart } from 'generated/proto/command_ready_start_pb';
import type { Command_Mulligan } from 'generated/proto/command_mulligan_pb';
import type { Command_DeckSelect } from 'generated/proto/command_deck_select_pb';
import type { MoveCard_ToZone } from 'generated/proto/move_card_to_zone_pb';
import type { Command_SetSideboardPlan } from 'generated/proto/command_set_sideboard_plan_pb';
import type { Command_SetSideboardLock } from 'generated/proto/command_set_sideboard_lock_pb';
import type { Command_SetActivePhase } from 'generated/proto/command_set_active_phase_pb';
import type { Command_GameSay } from 'generated/proto/command_game_say_pb';
import type { Event_GameStateChanged } from 'generated/proto/event_game_state_changed_pb';
import type { Event_GameSay } from 'generated/proto/event_game_say_pb';
import type { Event_MoveCard } from 'generated/proto/event_move_card_pb';
import type { Event_FlipCard } from 'generated/proto/event_flip_card_pb';
import type { Event_DestroyCard } from 'generated/proto/event_destroy_card_pb';
import type { Event_AttachCard } from 'generated/proto/event_attach_card_pb';
import type { Event_CreateToken } from 'generated/proto/event_create_token_pb';
import type { Event_SetCardAttr } from 'generated/proto/event_set_card_attr_pb';
import type { Event_SetCardCounter } from 'generated/proto/event_set_card_counter_pb';
import type { Event_CreateArrow } from 'generated/proto/event_create_arrow_pb';
import type { Event_DeleteArrow } from 'generated/proto/event_delete_arrow_pb';
import type { Event_CreateCounter } from 'generated/proto/event_create_counter_pb';
import type { Event_SetCounter } from 'generated/proto/event_set_counter_pb';
import type { Event_DelCounter } from 'generated/proto/event_del_counter_pb';
import type { Event_DrawCards } from 'generated/proto/event_draw_cards_pb';
import type { Event_RevealCards } from 'generated/proto/event_reveal_cards_pb';
import type { Event_Shuffle } from 'generated/proto/event_shuffle_pb';
import type { Event_RollDie } from 'generated/proto/event_roll_die_pb';
import type { Event_DumpZone } from 'generated/proto/event_dump_zone_pb';
import type { Event_ChangeZoneProperties } from 'generated/proto/event_change_zone_properties_pb';
import type { Event_SetActivePlayer } from 'generated/proto/event_set_active_player_pb';
import type { Event_SetActivePhase } from 'generated/proto/event_set_active_phase_pb';
import type { Event_ReverseTurn } from 'generated/proto/event_reverse_turn_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';
import type { color } from 'generated/proto/color_pb';
import type { ServerInfo_CardCounter } from 'generated/proto/serverinfo_cardcounter_pb';
import type { ServerInfo_Card } from 'generated/proto/serverinfo_card_pb';
import type { ServerInfo_Zone } from 'generated/proto/serverinfo_zone_pb';
import type { ServerInfo_Counter } from 'generated/proto/serverinfo_counter_pb';
import type { ServerInfo_Arrow } from 'generated/proto/serverinfo_arrow_pb';
import type { ServerInfo_PlayerProperties } from 'generated/proto/serverinfo_playerproperties_pb';
import type { ServerInfo_Player } from 'generated/proto/serverinfo_player_pb';

// ── Enum re-exports from generated proto files ────────────────────────────────

export { CardAttribute } from 'generated/proto/card_attributes_pb';
export { ServerInfo_Zone_ZoneType as ZoneType } from 'generated/proto/serverinfo_zone_pb';

// ── Proto utility types ───────────────────────────────────────────────────────

/**
 * Init shape for constructing protobuf messages via create().
 * Strips $typeName and $unknown branding, making all fields optional.
 * Use for function parameters that feed into create().
 */
export type ProtoInit<T> = {
  [K in keyof T as K extends '$typeName' | '$unknown' ? never : K]?: T[K];
};

// ── UI types (not proto mirrors) ──────────────────────────────────────────────

export type Game = ServerInfo_Game & {
  gameType: string;
};

export enum GameSortField {
  START_TIME = 'startTime'
}

export interface GameConfig {
  description: string;
  password: string;
  maxPlayers: number;
  onlyBuddies: boolean;
  onlyRegistered: boolean;
  spectatorsAllowed: boolean;
  spectatorsNeedPassword: boolean;
  spectatorsCanTalk: boolean;
  spectatorsSeeEverything: boolean;
  gameTypeIds: number[];
  joinAsJudge: boolean;
  joinAsSpectator: boolean;
  startingLifeTotal?: number;
  shareDecklistsOnLoad?: boolean;
}

export interface JoinGameParams {
  gameId: number;
  password: string;
  spectator: boolean;
  overrideRestrictions: boolean;
  joinAsJudge: boolean;
}

export enum LeaveGameReason {
  OTHER = 1,
  USER_KICKED = 2,
  USER_LEFT = 3,
  USER_DISCONNECTED = 4
}

// ── Type aliases for generated state mirror types ─────────────────────────────

export type Color = color;
export type CardCounterInfo = ServerInfo_CardCounter;
export type CardInfo = ServerInfo_Card;
export type ZoneInfo = ServerInfo_Zone;
export type CounterInfo = ServerInfo_Counter;
export type ArrowInfo = ServerInfo_Arrow;
export type PlayerProperties = ServerInfo_PlayerProperties;
export type PlayerInfo = ServerInfo_Player;

// ── Type aliases for generated event data types ───────────────────────────────

export type GameStateChangedData = Event_GameStateChanged;
export type GameSayData = Event_GameSay;
export type MoveCardData = Event_MoveCard;
export type FlipCardData = Event_FlipCard;
export type DestroyCardData = Event_DestroyCard;
export type AttachCardData = Event_AttachCard;
export type CreateTokenData = Event_CreateToken;
export type SetCardAttrData = Event_SetCardAttr;
export type SetCardCounterData = Event_SetCardCounter;
export type CreateArrowData = Event_CreateArrow;
export type DeleteArrowData = Event_DeleteArrow;
export type CreateCounterData = Event_CreateCounter;
export type SetCounterData = Event_SetCounter;
export type DelCounterData = Event_DelCounter;
export type DrawCardsData = Event_DrawCards;
export type RevealCardsData = Event_RevealCards;
export type ShuffleData = Event_Shuffle;
export type RollDieData = Event_RollDie;
export type DumpZoneData = Event_DumpZone;
export type ChangeZonePropertiesData = Event_ChangeZoneProperties;
export type SetActivePlayerData = Event_SetActivePlayer;
export type SetActivePhaseData = Event_SetActivePhase;
export type ReverseTurnData = Event_ReverseTurn;

// ── GameEventContext (re-export of generated type) ────────────────────────────

export type { GameEventContext };

/**
 * Passed to every game event handler alongside the event payload.
 * Contains per-container metadata from GameEventContainer.
 * Not stored in Redux — transient routing metadata only.
 */
export interface GameEventMeta {
  gameId: number;
  playerId: number;
  /** Raw protobuf GameEventContext object. Not stored in Redux. */
  context: GameEventContext | null;
  secondsElapsed: number;
  /** Proto type is uint32. Non-zero means the action was forced by a judge. */
  forcedByJudge: number;
}

// ── Type aliases for generated command param types (init shapes) ──────────────
// These use ProtoInit<> because callers construct plain objects;
// the command functions internally call create(Schema, params).

export type { CardToMove };
export type MoveCardParams = ProtoInit<Command_MoveCard>;
export type DrawCardsParams = ProtoInit<Command_DrawCards>;
export type RollDieParams = ProtoInit<Command_RollDie>;
export type ShuffleParams = ProtoInit<Command_Shuffle>;
export type FlipCardParams = ProtoInit<Command_FlipCard>;
export type AttachCardParams = ProtoInit<Command_AttachCard>;
export type CreateTokenParams = ProtoInit<Command_CreateToken>;
export type SetCardAttrParams = ProtoInit<Command_SetCardAttr>;
export type SetCardCounterParams = ProtoInit<Command_SetCardCounter>;
export type IncCardCounterParams = ProtoInit<Command_IncCardCounter>;
export type RevealCardsParams = ProtoInit<Command_RevealCards>;
export type DumpZoneParams = ProtoInit<Command_DumpZone>;
export type ChangeZonePropertiesParams = ProtoInit<Command_ChangeZoneProperties>;
export type CreateArrowParams = ProtoInit<Command_CreateArrow>;
export type DeleteArrowParams = ProtoInit<Command_DeleteArrow>;
export type CreateCounterParams = ProtoInit<Command_CreateCounter>;
export type SetCounterParams = ProtoInit<Command_SetCounter>;
export type IncCounterParams = ProtoInit<Command_IncCounter>;
export type DelCounterParams = ProtoInit<Command_DelCounter>;
export type KickFromGameParams = ProtoInit<Command_KickFromGame>;
export type ReadyStartParams = ProtoInit<Command_ReadyStart>;
export type MulliganParams = ProtoInit<Command_Mulligan>;
export type DeckSelectParams = ProtoInit<Command_DeckSelect>;
export type MoveCardToZone = MoveCard_ToZone;
export type SetSideboardPlanParams = ProtoInit<Command_SetSideboardPlan>;
export type SetSideboardLockParams = ProtoInit<Command_SetSideboardLock>;
export type SetActivePhaseParams = ProtoInit<Command_SetActivePhase>;
export type GameSayParams = ProtoInit<Command_GameSay>;
