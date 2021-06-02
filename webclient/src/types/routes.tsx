export enum RouteEnum {
  PLAYER = "/player/:name",
  SERVER = "/server",
  ROOM = "/room/:roomId",
  LOGS = "/logs",
  GAME = "/game",
  DECKS = "/decks",
  DECK = "/deck",
  ACCOUNT = "/account",
}

export function routeWithParams(route: RouteEnum, params: any) {
  let _route: string = route;

  Object.keys(params).forEach((param) => {
    _route = _route.replace(`:${param}`, params[param]);
  });

  return _route;
}
