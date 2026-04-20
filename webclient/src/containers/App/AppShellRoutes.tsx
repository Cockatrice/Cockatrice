import { Route, Routes } from 'react-router-dom';

import { App } from '@app/types';
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
} from '..';

const AppShellRoutes = () => (
  <div className="AppShell-routes overflow-scroll">
    <Routes>
      <Route path='*' element={<Initialize />} />

      <Route path={App.RouteEnum.ACCOUNT} element={<Account />} />
      <Route path={App.RouteEnum.DECKS} element={<Decks />} />
      <Route path={App.RouteEnum.GAME} element={<Game />} />
      <Route path={App.RouteEnum.LOGS} element={<Logs />} />
      <Route path={App.RouteEnum.PLAYER} element={<Player />} />
      {<Route path={App.RouteEnum.ROOM} element={<Room />} />}
      <Route path={App.RouteEnum.SERVER} element={<Server />} />
      <Route path={App.RouteEnum.LOGIN} element={<Login />} />
      <Route path={App.RouteEnum.UNSUPPORTED} element={<Unsupported />} />
    </Routes>
  </div>
);

export default AppShellRoutes;
