import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import {
  type RegistryEntry,
  type GameEvent,
  makeEntry,
  Event_Join_ext,
  Event_Leave_ext,
  Event_GameClosed_ext,
  Event_GameHostChanged_ext,
  Event_Kicked_ext,
  Event_GameStateChanged_ext,
  Event_PlayerPropertiesChanged_ext,
  Event_GameSay_ext,
  Event_CreateArrow_ext,
  Event_DeleteArrow_ext,
  Event_CreateCounter_ext,
  Event_SetCounter_ext,
  Event_DelCounter_ext,
  Event_DrawCards_ext,
  Event_RevealCards_ext,
  Event_Shuffle_ext,
  Event_RollDie_ext,
  Event_MoveCard_ext,
  Event_FlipCard_ext,
  Event_DestroyCard_ext,
  Event_AttachCard_ext,
  Event_CreateToken_ext,
  Event_SetCardAttr_ext,
  Event_SetCardCounter_ext,
  Event_SetActivePlayer_ext,
  Event_SetActivePhase_ext,
  Event_DumpZone_ext,
  Event_ChangeZoneProperties_ext,
  Event_ReverseTurn_ext,
} from '@app/generated';

import type { GameEventMeta } from '../../interfaces/WebSocketConfig';

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

type GameRegistryEntry<V = unknown> = RegistryEntry<V, GameEvent, GameEventMeta>;
export type GameExtensionRegistry = GameRegistryEntry[];

function makeGameEntry<V>(
  ext: GenExtension<GameEvent, V>,
  handler: (value: V, meta: GameEventMeta) => void,
): GameRegistryEntry {
  return makeEntry(ext, handler);
}

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
