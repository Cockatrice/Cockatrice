// eslint-disable-next-line
import React, { Component } from "react";

import { AuthGuard } from 'components/index';
import Layout from 'containers/Layout/Layout';

import './Decks.css';

class Decks extends Component {
  render() {
    return (
      <Layout>
        <AuthGuard />
        <span>"Decks"</span>
      </Layout>
    )
  }
}

export default Decks;
