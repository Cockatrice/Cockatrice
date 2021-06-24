// eslint-disable-next-line
import React, { Component } from "react";

import { AuthGuard } from "components";

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