import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_JoinGame_ext, Command_JoinGameSchema, Response_ResponseCode } from '@app/generated';
import type { JoinGameParams } from '@app/generated';

// Desktop message strings from cockatrice/src/interface/widgets/server/game_selector.cpp:234-260
// (GameSelector::checkResponse). Codes not listed here (e.g. RespContextError) intentionally
// fall through to the desktop's `default:;` — silent to the user.
const ERROR_MESSAGES: Record<number, string> = {
  [Response_ResponseCode.RespNotInRoom]: 'Please join the appropriate room first.',
  [Response_ResponseCode.RespNameNotFound]: 'The game does not exist any more.',
  [Response_ResponseCode.RespGameFull]: 'The game is already full.',
  [Response_ResponseCode.RespWrongPassword]: 'Wrong password.',
  [Response_ResponseCode.RespSpectatorsNotAllowed]: 'Spectators are not allowed in this game.',
  [Response_ResponseCode.RespOnlyBuddies]: 'This game is only open to its creator\'s buddies.',
  [Response_ResponseCode.RespUserLevelTooLow]: 'This game is only open to registered users.',
  [Response_ResponseCode.RespInIgnoreList]: 'You are being ignored by the creator of this game.',
};

export function joinGame(roomId: number, joinGameParams: JoinGameParams): void {
  const response = WebClient.instance.response.room;
  response.setJoinGamePending(true);

  const onResponseCode: { [code: number]: () => void } = {
    // Match desktop default:; — acknowledge silently, no user dialog.
    [Response_ResponseCode.RespContextError]: () => response.setJoinGamePending(false),
  };
  for (const codeStr of Object.keys(ERROR_MESSAGES)) {
    const code = Number(codeStr);
    onResponseCode[code] = () => response.setJoinGameError(code, ERROR_MESSAGES[code]);
  }

  WebClient.instance.protobuf.sendRoomCommand(
    roomId,
    Command_JoinGame_ext,
    create(Command_JoinGameSchema, joinGameParams),
    {
      onSuccess: () => {
        response.setJoinGamePending(false);
        response.joinedGame(roomId, joinGameParams.gameId);
      },
      onResponseCode,
      onError: () => response.setJoinGamePending(false),
    },
  );
}
