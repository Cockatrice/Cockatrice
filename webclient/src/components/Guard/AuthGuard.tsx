import { Navigate } from 'react-router-dom';

import { ServerSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

const AuthGuard = () => {
  const isConnected = useAppSelector(ServerSelectors.getIsConnected);
  return !isConnected
    ? <Navigate to={App.RouteEnum.LOGIN} />
    : <></>;
};

export default AuthGuard;
