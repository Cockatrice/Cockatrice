  import React from "react";
import { Redirect, Route, Switch } from "react-router-dom";

import { RouteEnum } from "types";
import { Account } from "containers";
import { 
  Decks, 
  Game, 
  Player, 
  Room, 
  Server, 
  Logs } from "components";

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