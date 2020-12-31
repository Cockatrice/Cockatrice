import React, { Component } from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";

import { Selectors } from "store/server";
import { User } from "types";

import { AuthenticationService } from "AppShell/common/services";
import { RouteEnum } from "AppShell/common/types";

class ModGuard extends Component<ModGuardProps> {
  render() {
    return !AuthenticationService.isModerator(this.props.user)
      ? <Redirect from="*" to={RouteEnum.SERVER} />
      : "";
  }
};

interface ModGuardProps {
  user: User;
}

const mapStateToProps = state => ({
  user: Selectors.getUser(state),
});

export default connect(mapStateToProps)(ModGuard);