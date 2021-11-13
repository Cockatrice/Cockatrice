import React from 'react';
import { Redirect, Route, Switch } from 'react-router-dom';

import { RouteEnum } from 'types';
import {
  Account,
  Decks,
  Game,
  Player,
  Room,
  Server,
  Login,
  Logs
} from 'containers';

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
      <Route path={RouteEnum.LOGIN} render={() => <Login />} />

      <Redirect from="*" to={RouteEnum.LOGIN} />
    </Switch>
  </div>
);

export default Routes;
