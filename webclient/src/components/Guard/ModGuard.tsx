import React, { Component } from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";

import { ServerSelectors } from "store";
import { User } from "types";

import { AuthenticationService } from "websocket";
import { RouteEnum } from "types";

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
  user: ServerSelectors.getUser(state),
});

export default connect(mapStateToProps)(ModGuard);