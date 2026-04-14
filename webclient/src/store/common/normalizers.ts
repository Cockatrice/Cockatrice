import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';
import type { ServerInfo_GameType } from 'generated/proto/serverinfo_gametype_pb';
import { Game, GametypeMap, LogItem, LogGroups, Message, Room } from 'types';

/** Flatten a gametype list into a lookup map of { gameTypeId → description }. */
export function normalizeGametypeMap(gametypeList: ServerInfo_GameType[]): GametypeMap {
  return gametypeList.reduce<GametypeMap>((map, type) => {
    map[type.gameTypeId] = type.description;
    return map;
  }, {});
}

/** Flatten room gameTypes into a map object and normalize all games inside. */
export function normalizeRoomInfo(roomInfo: ServerInfo_Room): Room {
  const gametypeMap = normalizeGametypeMap(roomInfo.gametypeList);

  const gameList = roomInfo.gameList.map(
    (game) => normalizeGameObject(game, gametypeMap),
  );

  return {
    ...roomInfo,
    gametypeMap,
    gameList,
    order: 0,
  };
}

/** Flatten gameTypes[] into a gameType string; fill in default sortable values. */
export function normalizeGameObject(game: ServerInfo_Game, gametypeMap: GametypeMap): Game {
  const { gameTypes, description } = game;
  const hasType = gameTypes && gameTypes.length;

  return {
    ...game,
    gameType: hasType ? gametypeMap[gameTypes[0]] : '',
    description: description || '',
  };
}

/** Group a flat LogItem[] into { room, game, chat } buckets for the server store. */
export function normalizeLogs(logs: LogItem[]): LogGroups {
  return logs.reduce((obj, log) => {
    const type = log.targetType as keyof LogGroups;
    obj[type] = obj[type] || [];
    obj[type]!.push(log);
    return obj;
  }, {} as LogGroups);
}

/**
 * Prepend "name: " to the message text when a sender name is present.
 * Messages from the current user are sent without a name by the server,
 * so this is a no-op for those.
 * Returns a new Message — does not mutate the original.
 */
export function normalizeUserMessage(message: Message): Message {
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
