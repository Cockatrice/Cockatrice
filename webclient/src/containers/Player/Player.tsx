// eslint-disable-next-line
import React, { Component } from "react";
import Layout from 'containers/Layout/Layout';

import { AuthGuard } from 'components';

class Player extends Component {
  render() {
    return (
      <Layout>
        <AuthGuard />
        <span>"Player"</span>
      </Layout>
    )
  }
}

export default Player;
