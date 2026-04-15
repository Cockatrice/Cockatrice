import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { Data, Enriched } from '@app/types';

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

type GameRegistryEntry<V = unknown> = Data.RegistryEntry<V, Data.GameEvent, Enriched.GameEventMeta>;
export type GameExtensionRegistry = GameRegistryEntry[];

function makeGameEntry<V>(
  ext: GenExtension<Data.GameEvent, V>,
  handler: (value: V, meta: Enriched.GameEventMeta) => void,
): GameRegistryEntry {
  return Data.makeEntry(ext, handler);
}

export const GameEvents: GameExtensionRegistry = [
  makeGameEntry(Data.Event_Join_ext, joinGame),
  makeGameEntry(Data.Event_Leave_ext, leaveGame),
  makeGameEntry(Data.Event_GameClosed_ext, gameClosed),
  makeGameEntry(Data.Event_GameHostChanged_ext, gameHostChanged),
  makeGameEntry(Data.Event_Kicked_ext, kicked),
  makeGameEntry(Data.Event_GameStateChanged_ext, gameStateChanged),
  makeGameEntry(Data.Event_PlayerPropertiesChanged_ext, playerPropertiesChanged),
  makeGameEntry(Data.Event_GameSay_ext, gameSay),
  makeGameEntry(Data.Event_CreateArrow_ext, createArrow),
  makeGameEntry(Data.Event_DeleteArrow_ext, deleteArrow),
  makeGameEntry(Data.Event_CreateCounter_ext, createCounter),
  makeGameEntry(Data.Event_SetCounter_ext, setCounter),
  makeGameEntry(Data.Event_DelCounter_ext, delCounter),
  makeGameEntry(Data.Event_DrawCards_ext, drawCards),
  makeGameEntry(Data.Event_RevealCards_ext, revealCards),
  makeGameEntry(Data.Event_Shuffle_ext, shuffle),
  makeGameEntry(Data.Event_RollDie_ext, rollDie),
  makeGameEntry(Data.Event_MoveCard_ext, moveCard),
  makeGameEntry(Data.Event_FlipCard_ext, flipCard),
  makeGameEntry(Data.Event_DestroyCard_ext, destroyCard),
  makeGameEntry(Data.Event_AttachCard_ext, attachCard),
  makeGameEntry(Data.Event_CreateToken_ext, createToken),
  makeGameEntry(Data.Event_SetCardAttr_ext, setCardAttr),
  makeGameEntry(Data.Event_SetCardCounter_ext, setCardCounter),
  makeGameEntry(Data.Event_SetActivePlayer_ext, setActivePlayer),
  makeGameEntry(Data.Event_SetActivePhase_ext, setActivePhase),
  makeGameEntry(Data.Event_DumpZone_ext, dumpZone),
  makeGameEntry(Data.Event_ChangeZoneProperties_ext, changeZoneProperties),
  makeGameEntry(Data.Event_ReverseTurn_ext, reverseTurn),
];
