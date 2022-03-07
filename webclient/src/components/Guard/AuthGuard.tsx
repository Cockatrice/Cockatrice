import React from 'react';
import { connect } from 'react-redux';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from 'store';
import { RouteEnum } from 'types';

import { AuthenticationService } from 'api';

const AuthGuard = ({ state }: AuthGuardProps) => {
  return !AuthenticationService.isConnected(state)
    ? <Navigate to={RouteEnum.LOGIN} />
    : <div></div>;
};

interface AuthGuardProps {
  state: number;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
});

export default connect(mapStateToProps)(AuthGuard);
