// eslint-disable-next-line
import React, { Component } from "react";
import Layout from '../Layout/Layout';

import { AuthGuard } from '@app/components';

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
