import { Data, Enriched } from '@app/types';

export function normalizeGametypeMap(gametypeList: Data.ServerInfo_GameType[]): Enriched.GametypeMap {
  return gametypeList.reduce<Enriched.GametypeMap>((map, type) => {
    map[type.gameTypeId] = type.description;
    return map;
  }, {});
}

export function normalizeRoomInfo(roomInfo: Data.ServerInfo_Room): Enriched.Room {
  const gametypeMap = normalizeGametypeMap(roomInfo.gametypeList);

  const games: { [gameId: number]: Enriched.Game } = {};
  for (const rawGame of roomInfo.gameList) {
    const normalized = normalizeGameObject(rawGame, gametypeMap);
    games[normalized.info.gameId] = normalized;
  }

  const users: { [userName: string]: Data.ServerInfo_User } = {};
  for (const user of roomInfo.userList) {
    users[user.name] = user;
  }

  return {
    info: roomInfo,
    gametypeMap,
    order: 0,
    games,
    users,
  };
}

export function normalizeGameObject(game: Data.ServerInfo_Game, gametypeMap: Enriched.GametypeMap): Enriched.Game {
  const { gameTypes } = game;
  const hasType = gameTypes && gameTypes.length;
  return {
    info: game,
    gameType: hasType ? (gametypeMap[gameTypes[0]] ?? '') : '',
  };
}

export function normalizeLogs(logs: Data.ServerInfo_ChatMessage[]): Enriched.LogGroups {
  return logs.reduce<Enriched.LogGroups>((obj, log) => {
    const type = log.targetType as keyof Enriched.LogGroups;
    if (obj[type]) {
      obj[type].push(log);
    }
    return obj;
  }, { room: [], game: [], chat: [] });
}

// @critical Server omits `name` on messages from the current user; preserves that as a no-op.
// See .github/instructions/webclient.instructions.md#protocol-quirks.
export function normalizeUserMessage(message: Enriched.Message): Enriched.Message {
  if (!message.name) {
    return message;
  }
  return { ...message, message: `${message.name}: ${message.message}` };
}

// endTime is epoch ms; 0 means permanent.
export function normalizeBannedUserError(reason: string, endTime: number): string {
  let error: string;

  if (endTime) {
    error = 'You are banned until ' + new Date(endTime).toString();
  } else {
    error = 'You are permanently banned';
  }

  if (reason) {
    error += '\n\n' + reason;
  }

  return error;
}
