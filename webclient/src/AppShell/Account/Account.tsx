// eslint-disable-next-line
import React, { Component } from 'react';
import Button from '@material-ui/core/Button';

import { AuthenticationService } from 'AppShell/common/services';
import AuthGuard from 'AppShell/common/guards/AuthGuard';

import './Account.css';

class Account extends Component {
  render() {
    return (
      <div>
        <AuthGuard />
        <div>"Account"</div>
        <Button color="primary" variant="contained" onClick={() => AuthenticationService.disconnect()}>Logout</Button>
      </div>  
    )
  }
}

export default Account;