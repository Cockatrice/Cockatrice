  import React from 'react';
import { Redirect, Route, Switch } from "react-router-dom";

import { RouteEnum } from './common/types';

import Account from './Account/Account';
import Decks from './Decks/Decks';
import Game from './Game/Game';
import Logs from './Logs/Logs';
import Room from './Room/Room';
import Server from './Server/Server';

const Routes = () => (
  <div className="AppShell-routes overflow-scroll">
    <Switch>
      <Route path={RouteEnum.SERVER} render={() => <Server />} />
      {<Route path={RouteEnum.ROOM} render={() => <Room />} />}
      <Route path={RouteEnum.LOGS} render={() => <Logs />} />
      <Route path={RouteEnum.GAME} render={() => <Game />} />
      <Route path={RouteEnum.DECKS} render={() => <Decks />} />
      {/*<Route path={RouteEnum.DECK} render={() => <Deck />} />*/}
      <Route path={RouteEnum.ACCOUNT} render={() => <Account />} />
      <Redirect from="/" to={RouteEnum.SERVER} />
    </Switch>
  </div>
);

export default Routes;