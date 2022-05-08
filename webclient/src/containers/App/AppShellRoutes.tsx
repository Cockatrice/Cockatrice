import React from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { RouteEnum } from 'types';
import {
  Account,
  Decks,
  Game,
  Player,
  Room,
  Server,
  Login,
  Logs,
  Initialize,
  Unsupported
} from 'containers';

const AppShellRoutes = () => (
  <div className="AppShell-routes overflow-scroll">
    <Routes>
      <Route path='*' element={<Initialize />} />

      <Route path={RouteEnum.ACCOUNT} element={<Account />} />
      <Route path={RouteEnum.DECKS} element={<Decks />} />
      <Route path={RouteEnum.GAME} element={<Game />} />
      <Route path={RouteEnum.LOGS} element={<Logs />} />
      <Route path={RouteEnum.PLAYER} element={<Player />} />
      {<Route path={RouteEnum.ROOM} element={<Room />} />}
      <Route path={RouteEnum.SERVER} element={<Server />} />
      <Route path={RouteEnum.LOGIN} element={<Login />} />
      <Route path={RouteEnum.UNSUPPORTED} element={<Unsupported />} />
    </Routes>
  </div>
);

export default AppShellRoutes;
