import React from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";

import { ServerSelectors } from "store";
import { RouteEnum } from "types";

import { AuthenticationService } from "api";

const AuthGuard = ({ state }: AuthGuardProps) => {
  return !AuthenticationService.isConnected(state)
    ? <Redirect from="*" to={RouteEnum.SERVER} />
    : <div></div>;
};

interface AuthGuardProps {
  state: number;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
});

export default connect(mapStateToProps)(AuthGuard);
