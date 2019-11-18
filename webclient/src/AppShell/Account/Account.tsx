// eslint-disable-next-line
import React, { Component } from 'react';

import { AuthGuard } from 'AppShell/common/guards';

import './Account.css';

class Account extends Component {
  render() {
    return (
      <div>
        <AuthGuard />
        <span>"Account"</span>
      </div>  
    )
  }
}

export default Account;