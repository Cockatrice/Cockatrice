import { GameExtensionRegistry, makeGameEntry } from '../../services/protobuf-types';
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

import { Event_Join_ext } from 'generated/proto/event_join_pb';
import { Event_Leave_ext } from 'generated/proto/event_leave_pb';
import { Event_GameClosed_ext } from 'generated/proto/event_game_closed_pb';
import { Event_GameHostChanged_ext } from 'generated/proto/event_game_host_changed_pb';
import { Event_Kicked_ext } from 'generated/proto/event_kicked_pb';
import { Event_GameStateChanged_ext } from 'generated/proto/event_game_state_changed_pb';
import { Event_PlayerPropertiesChanged_ext } from 'generated/proto/event_player_properties_changed_pb';
import { Event_GameSay_ext } from 'generated/proto/event_game_say_pb';
import { Event_CreateArrow_ext } from 'generated/proto/event_create_arrow_pb';
import { Event_DeleteArrow_ext } from 'generated/proto/event_delete_arrow_pb';
import { Event_CreateCounter_ext } from 'generated/proto/event_create_counter_pb';
import { Event_SetCounter_ext } from 'generated/proto/event_set_counter_pb';
import { Event_DelCounter_ext } from 'generated/proto/event_del_counter_pb';
import { Event_DrawCards_ext } from 'generated/proto/event_draw_cards_pb';
import { Event_RevealCards_ext } from 'generated/proto/event_reveal_cards_pb';
import { Event_Shuffle_ext } from 'generated/proto/event_shuffle_pb';
import { Event_RollDie_ext } from 'generated/proto/event_roll_die_pb';
import { Event_MoveCard_ext } from 'generated/proto/event_move_card_pb';
import { Event_FlipCard_ext } from 'generated/proto/event_flip_card_pb';
import { Event_DestroyCard_ext } from 'generated/proto/event_destroy_card_pb';
import { Event_AttachCard_ext } from 'generated/proto/event_attach_card_pb';
import { Event_CreateToken_ext } from 'generated/proto/event_create_token_pb';
import { Event_SetCardAttr_ext } from 'generated/proto/event_set_card_attr_pb';
import { Event_SetCardCounter_ext } from 'generated/proto/event_set_card_counter_pb';
import { Event_SetActivePlayer_ext } from 'generated/proto/event_set_active_player_pb';
import { Event_SetActivePhase_ext } from 'generated/proto/event_set_active_phase_pb';
import { Event_DumpZone_ext } from 'generated/proto/event_dump_zone_pb';
import { Event_ChangeZoneProperties_ext } from 'generated/proto/event_change_zone_properties_pb';
import { Event_ReverseTurn_ext } from 'generated/proto/event_reverse_turn_pb';

export const GameEvents: GameExtensionRegistry = [
  makeGameEntry(Event_Join_ext, joinGame),
  makeGameEntry(Event_Leave_ext, leaveGame),
  makeGameEntry(Event_GameClosed_ext, gameClosed),
  makeGameEntry(Event_GameHostChanged_ext, gameHostChanged),
  makeGameEntry(Event_Kicked_ext, kicked),
  makeGameEntry(Event_GameStateChanged_ext, gameStateChanged),
  makeGameEntry(Event_PlayerPropertiesChanged_ext, playerPropertiesChanged),
  makeGameEntry(Event_GameSay_ext, gameSay),
  makeGameEntry(Event_CreateArrow_ext, createArrow),
  makeGameEntry(Event_DeleteArrow_ext, deleteArrow),
  makeGameEntry(Event_CreateCounter_ext, createCounter),
  makeGameEntry(Event_SetCounter_ext, setCounter),
  makeGameEntry(Event_DelCounter_ext, delCounter),
  makeGameEntry(Event_DrawCards_ext, drawCards),
  makeGameEntry(Event_RevealCards_ext, revealCards),
  makeGameEntry(Event_Shuffle_ext, shuffle),
  makeGameEntry(Event_RollDie_ext, rollDie),
  makeGameEntry(Event_MoveCard_ext, moveCard),
  makeGameEntry(Event_FlipCard_ext, flipCard),
  makeGameEntry(Event_DestroyCard_ext, destroyCard),
  makeGameEntry(Event_AttachCard_ext, attachCard),
  makeGameEntry(Event_CreateToken_ext, createToken),
  makeGameEntry(Event_SetCardAttr_ext, setCardAttr),
  makeGameEntry(Event_SetCardCounter_ext, setCardCounter),
  makeGameEntry(Event_SetActivePlayer_ext, setActivePlayer),
  makeGameEntry(Event_SetActivePhase_ext, setActivePhase),
  makeGameEntry(Event_DumpZone_ext, dumpZone),
  makeGameEntry(Event_ChangeZoneProperties_ext, changeZoneProperties),
  makeGameEntry(Event_ReverseTurn_ext, reverseTurn),
];

