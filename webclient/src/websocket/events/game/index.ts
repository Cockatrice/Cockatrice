import { ProtobufEvents } from '../../services/ProtobufService';
import { joinGame } from './joinGame';
import { leaveGame } from './leaveGame';


export const GameEvents: ProtobufEvents = {
  '.Event_Join.ext': () => joinGame,
  '.Event_Leave.ext': () => leaveGame,
  '.Event_GameClosed.ext': () => console.log('Event_GameClosed.ext'),
  '.Event_GameHostChanged.ext': () => console.log('Event_GameHostChanged.ext'),
  '.Event_Kicked.ext': () => console.log('Event_Kicked.ext'),
  '.Event_GameStateChanged.ext': () => console.log('Event_GameStateChanged.ext'),
  // '.Event_PlayerPropertiesChanged.ext': () => console.log("Event_PlayerProperties.ext"),
  '.Event_GameSay.ext': () => console.log('Event_GameSay.ext'),
  '.Event_CreateArrow.ext': () => console.log('Event_CreateArrow.ext'),
  '.Event_DeleteArrow.ext': () => console.log('Event_DeleteArrow.ext'),
  '.Event_CreateCounter.ext': () => console.log('Event_CreateCounter.ext'),
  '.Event_SetCounter.ext': () => console.log('Event_SetCounter.ext'),
  '.Event_DelCounter.ext': () => console.log('Event_DelCounter.ext'),
  '.Event_DrawCards.ext': () => console.log('Event_DrawCards.ext'),
  '.Event_RevealCards.ext': () => console.log('Event_RevealCards.ext'),
  '.Event_Shuffle.ext': () => console.log('Event_Shuffle.ext'),
  '.Event_RollDie.ext': () => console.log('Event_Roll.ext'),
  '.Event_MoveCard.ext': () => console.log('Event_MoveCard.ext'),
  '.Event_FlipCard.ext': () => console.log('Event_FlipCard.ext'),
  '.Event_DestroyCard.ext': () => console.log('Event_DestroyCard.ext'),
  '.Event_AttachCard.ext': () => console.log('Event_AttachCard.ext'),
  '.Event_CreateToken.ext': () => console.log('Event_CreateToken.ext'),
  '.Event_SetCardAttribute.ext': () => console.log('Event_SetCardAttribute.ext'),
  '.Event_SetCardCounter.ext': () => console.log('Event_SetCardCounter.ext'),
  '.Event_SetActivePlayer.ext': () => console.log('Event_SetActivePlayer.ext'),
  '.Event_SetActivePhase.ext': () => console.log('Event_SetActivePhase.ext'),
  '.Event_DumpZone.ext': () => console.log('Event_DumpZone.ext'),
  '.Event_ChangeZoneProperties.ext': () => console.log('Event_ChangeZoneProperties.ext'),
  '.Event_ReverseTurn.ext': () => console.log('Event_ReverseTurn.ext'),
};
