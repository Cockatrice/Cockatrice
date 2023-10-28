// eslint-disable-next-line
import React, { Component } from "react";

import { AuthGuard } from 'components';
import Layout from 'containers/Layout/Layout';

import './Game.css';

class Game extends Component {
  render() {
    return (
      <Layout>
        <AuthGuard />
        <span>"Game"</span>
      </Layout>
    )
  }
}

export default Game;
