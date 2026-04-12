import { ProtobufEvents } from '../../services/ProtobufService';
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

export const GameEvents: ProtobufEvents = {
  '.Event_Join.ext': joinGame,
  '.Event_Leave.ext': leaveGame,
  '.Event_GameClosed.ext': gameClosed,
  '.Event_GameHostChanged.ext': gameHostChanged,
  '.Event_Kicked.ext': kicked,
  '.Event_GameStateChanged.ext': gameStateChanged,
  '.Event_PlayerPropertiesChanged.ext': playerPropertiesChanged,
  '.Event_GameSay.ext': gameSay,
  '.Event_CreateArrow.ext': createArrow,
  '.Event_DeleteArrow.ext': deleteArrow,
  '.Event_CreateCounter.ext': createCounter,
  '.Event_SetCounter.ext': setCounter,
  '.Event_DelCounter.ext': delCounter,
  '.Event_DrawCards.ext': drawCards,
  '.Event_RevealCards.ext': revealCards,
  '.Event_Shuffle.ext': shuffle,
  '.Event_RollDie.ext': rollDie,
  '.Event_MoveCard.ext': moveCard,
  '.Event_FlipCard.ext': flipCard,
  '.Event_DestroyCard.ext': destroyCard,
  '.Event_AttachCard.ext': attachCard,
  '.Event_CreateToken.ext': createToken,
  '.Event_SetCardAttr.ext': setCardAttr,
  '.Event_SetCardCounter.ext': setCardCounter,
  '.Event_SetActivePlayer.ext': setActivePlayer,
  '.Event_SetActivePhase.ext': setActivePhase,
  '.Event_DumpZone.ext': dumpZone,
  '.Event_ChangeZoneProperties.ext': changeZoneProperties,
  '.Event_ReverseTurn.ext': reverseTurn,
};
