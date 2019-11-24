// eslint-disable-next-line
import React, { Component } from "react";

import AuthGuard from "AppShell/common/guards/AuthGuard";

import "./Game.css";

class Game extends Component {
  render() {
    return (
      <div>
        <AuthGuard />
        <span>"Game"</span>
      </div>
    )
  }
}

export default Game;