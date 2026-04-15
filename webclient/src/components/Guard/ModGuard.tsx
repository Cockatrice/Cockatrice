import React from 'react';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from '@app/store';
import { AuthenticationService } from '@app/api';
import { App } from '@app/types';
import { useAppSelector } from '@app/store';

const ModGuard = () => {
  const user = useAppSelector(state => ServerSelectors.getUser(state));
  return !AuthenticationService.isModerator(user)
    ? <Navigate to={App.RouteEnum.SERVER} />
    : <></>;
};

export default ModGuard;
