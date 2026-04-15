// eslint-disable-next-line
import React, { Component } from "react";

import { AuthGuard } from '@app/components';
import Layout from '../Layout/Layout';

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
