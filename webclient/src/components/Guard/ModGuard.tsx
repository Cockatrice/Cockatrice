import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Navigate } from 'react-router-dom';

import { ServerSelectors } from 'store';
import { User } from 'types';

import { AuthenticationService } from 'api';
import { RouteEnum } from 'types';

class ModGuard extends Component<ModGuardProps> {
  render() {
    return !AuthenticationService.isModerator(this.props.user)
      ? <Navigate to={RouteEnum.SERVER} />
      : '';
  }
};

interface ModGuardProps {
  user: User;
}

const mapStateToProps = state => ({
  user: ServerSelectors.getUser(state),
});

export default connect(mapStateToProps)(ModGuard);
