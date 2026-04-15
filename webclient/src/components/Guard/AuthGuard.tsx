import React from 'react';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from '@app/store';
import { App } from '@app/types';
import { useAppSelector } from '@app/store';
import { AuthenticationService } from '@app/api';

const AuthGuard = () => {
  const state = useAppSelector(s => ServerSelectors.getState(s));
  return !AuthenticationService.isConnected(state)
    ? <Navigate to={App.RouteEnum.LOGIN} />
    : <div></div>;
};

export default AuthGuard;
