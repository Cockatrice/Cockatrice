  import React from "react";
import { Redirect, Route, Switch } from "react-router-dom";

import { RouteEnum } from "./common/types";

import Account from "./Account/Account";
import Decks from "./Decks/Decks";
import Game from "./Game/Game";
import Logs from "./Logs/Logs";
import Player from "./Player/Player";
import Room from "./Room/Room";
import Server from "./Server/Server";

const Routes = () => (
  <div className="AppShell-routes overflow-scroll">
    <Switch>
      <Route path={RouteEnum.ACCOUNT} render={() => <Account />} />
      <Route path={RouteEnum.DECKS} render={() => <Decks />} />
      <Route path={RouteEnum.GAME} render={() => <Game />} />
      <Route path={RouteEnum.LOGS} render={() => <Logs />} />
      <Route path={RouteEnum.PLAYER} render={() => <Player />} />
      {<Route path={RouteEnum.ROOM} render={() => <Room />} />}
      <Route path={RouteEnum.SERVER} render={() => <Server />} />
      <Redirect from="/" to={RouteEnum.SERVER} />
    </Switch>
  </div>
);

export default Routes;