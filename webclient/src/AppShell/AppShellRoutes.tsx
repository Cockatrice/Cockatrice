import React from 'react';
import { Redirect, Route, Switch } from "react-router-dom";

import RouteEnum from '../common/types/RouteEnum';
import AuthGuard from '../common/guards/AuthGuard/AuthGuard';

import Account from './Account/Account';
import Connect from './Connect/Connect';
import Decks from './Decks/Decks';
import Game from './Game/Game';
import MainRoom from './MainRoom/MainRoom';

const Routes = () => (
  <div>
    {/*<AuthGuard />*/}

  	<Switch>
      <Route path={RouteEnum.ACCOUNT} render={() => <Account />} />
      <Route path={RouteEnum.CONNECT} render={() => <Connect />} />
      <Route path={RouteEnum.DECKS} render={() => <Decks />} />
      <Route path={RouteEnum.GAME} render={() => <Game />} />
      <Route path={RouteEnum.MAIN} render={() => <MainRoom />} />
      <Redirect from="/" to={RouteEnum.MAIN} />
    </Switch>
  </div>
);

export default Routes;