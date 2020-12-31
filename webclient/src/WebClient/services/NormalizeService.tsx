export class NormalizeService {
  // Flatten room gameTypes into map object
  static normalizeRoomInfo(roomInfo) {
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
  static normalizeGameObject(game, gametypeMap) {
    const { gameTypes, description } = game;
    const hasType = gameTypes && gameTypes.length;
    game.gameType = hasType ? gametypeMap[gameTypes[0]] : "";

    game.description = description || "";
  }
  
  // Flatten logs[] into object mapped by targetType (room, game, chat)
  static normalizeLogs(logs) {
    return logs.reduce((obj, log) => {
      const { targetType } = log;
      obj[targetType] = obj[targetType] || [];
      obj[targetType].push(log);
      return obj;
    }, {});
  }

  // messages sent by current user dont have their username prepended
  static normalizeUserMessage(message) {
    const { name } = message;

    if (name) {
      message.message = `${name}: ${message.message}`;
    }
  }
}
