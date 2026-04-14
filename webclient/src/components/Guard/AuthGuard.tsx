import React from 'react';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from 'store';
import { RouteEnum } from 'types';
import { useAppSelector } from 'store/store';
import { AuthenticationService } from 'api';

const AuthGuard = () => {
  const state = useAppSelector(s => ServerSelectors.getState(s));
  return !AuthenticationService.isConnected(state)
    ? <Navigate to={RouteEnum.LOGIN} />
    : <div></div>;
};

export default AuthGuard;
