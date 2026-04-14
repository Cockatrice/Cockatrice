import React from 'react';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from 'store';
import { AuthenticationService } from 'api';
import { RouteEnum } from 'types';
import { useAppSelector } from 'store/store';

const ModGuard = () => {
  const user = useAppSelector(state => ServerSelectors.getUser(state));
  return !AuthenticationService.isModerator(user)
    ? <Navigate to={RouteEnum.SERVER} />
    : <></>;
};

export default ModGuard;
