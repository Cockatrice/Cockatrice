import React, { Component } from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";

import { ServerSelectors } from "store";
import { RouteEnum } from "types";

import { AuthenticationService } from "websocket";

class AuthGuard extends Component<AuthGuardProps> {
  render() {
    return !AuthenticationService.isConnected(this.props.state)
      ? <Redirect from="*" to={RouteEnum.SERVER} />
      : "";
  }
};

interface AuthGuardProps {
  state: number;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
});

export default connect(mapStateToProps)(AuthGuard);