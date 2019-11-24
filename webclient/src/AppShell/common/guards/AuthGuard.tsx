import React, { Component } from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";

import { Selectors } from "store/server";

import { AuthenticationService } from "AppShell/common/services";
import { RouteEnum } from "AppShell/common/types";

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
  state: Selectors.getState(state),
});

export default connect(mapStateToProps)(AuthGuard);