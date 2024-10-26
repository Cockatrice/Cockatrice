import { Game, GametypeMap, LogItem, LogGroups, Message, Room } from 'types';

export default class NormalizeService {
  // Flatten room gameTypes into map object
  static normalizeRoomInfo(roomInfo: Room): void {
    roomInfo.gametypeMap = {};

    const { gametypeList, gametypeMap, gameList } = roomInfo;

    gametypeList.reduce((map, type) => {
      map[type.gameTypeId] = type.description;
      return map;
    }, gametypeMap);

    gameList.forEach((game) => NormalizeService.normalizeGameObject(game, gametypeMap));
  }

  // Flatten gameTypes[] into gameType field
  // Default sortable values ("" || 0 || -1)
  static normalizeGameObject(game: Game, gametypeMap: GametypeMap): void {
    const { gameTypes, description } = game;
    const hasType = gameTypes && gameTypes.length;
    game.gameType = hasType ? gametypeMap[gameTypes[0]] : '';

    game.description = description || '';
  }

  // Flatten logs[] into object mapped by targetType (room, game, chat)
  static normalizeLogs(logs: LogItem[]): LogGroups {
    return logs.reduce((obj, log) => {
      const { targetType } = log;
      obj[targetType] = obj[targetType] || [];
      obj[targetType].push(log);
      return obj;
    }, {} as LogGroups);
  }

  // messages sent by current user dont have their username prepended
  static normalizeUserMessage(message: Message): void {
    const { name } = message;

    if (name) {
      message.message = `${name}: ${message.message}`;
    }
  }

  // Banned reason string is not being exposed by the server
  static normalizeBannedUserError(reasonStr: string, endTime: number): string {
    let error;

    if (endTime) {
      error = 'You are banned until ' + new Date(endTime).toString();
    } else {
      error = 'You are permanently banned';
    }

    if (reasonStr) {
      error += '\n\n' + reasonStr;
    }

    return error;
  }
}
