import { Data, Enriched } from '@app/types';

/** Flatten a gametype list into a lookup map of { gameTypeId → description }. */
export function normalizeGametypeMap(gametypeList: Data.ServerInfo_GameType[]): Enriched.GametypeMap {
  return gametypeList.reduce<Enriched.GametypeMap>((map, type) => {
    map[type.gameTypeId] = type.description;
    return map;
  }, {});
}

/**
 * Build an Enriched.Room (composition shape) from a raw proto. The proto is
 * stored verbatim on `info` and the repeated collections are normalized into
 * keyed maps alongside it. `info.gameList`, `info.userList`, `info.gametypeList`
 * are left as the wire snapshot — callers should always read the normalized
 * fields, never those.
 */
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

/** Wrap a raw ServerInfo_Game in the composition shape with cached gameType. */
export function normalizeGameObject(game: Data.ServerInfo_Game, gametypeMap: Enriched.GametypeMap): Enriched.Game {
  const { gameTypes } = game;
  const hasType = gameTypes && gameTypes.length;
  return {
    info: game,
    gameType: hasType ? (gametypeMap[gameTypes[0]] ?? '') : '',
  };
}

/** Group a flat LogItem[] into { room, game, chat } buckets for the server store. */
export function normalizeLogs(logs: Data.ServerInfo_ChatMessage[]): Enriched.LogGroups {
  return logs.reduce<Enriched.LogGroups>((obj, log) => {
    const type = log.targetType as keyof Enriched.LogGroups;
    if (obj[type]) {
      obj[type].push(log);
    }
    return obj;
  }, { room: [], game: [], chat: [] });
}

/**
 * Prepend "name: " to the message text when a sender name is present.
 * Messages from the current user are sent without a name by the server,
 * so this is a no-op for those.
 * Returns a new Message — does not mutate the original.
 */
export function normalizeUserMessage(message: Enriched.Message): Enriched.Message {
  if (!message.name) {
    return message;
  }
  return { ...message, message: `${message.name}: ${message.message}` };
}

/**
 * Build the user-facing ban error string from raw server data.
 * The server sends a reason string and an endTime epoch ms (0 = permanent).
 * Messages from the current user do not carry the username — this quirk is
 * handled at the dispatch layer so the redux store always stores a clean string.
 */
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
