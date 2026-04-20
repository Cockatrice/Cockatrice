import { Navigate } from 'react-router-dom';

import { ServerSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

const ModGuard = () => {
  const isModerator = useAppSelector(ServerSelectors.getIsUserModerator);
  return !isModerator
    ? <Navigate to={App.RouteEnum.SERVER} />
    : <></>;
};

export default ModGuard;
