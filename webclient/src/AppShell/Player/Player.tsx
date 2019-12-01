// eslint-disable-next-line
import React, { Component } from "react";

import AuthGuard from "AppShell/common/guards/AuthGuard";

import "./Player.css";

class Player extends Component {
  render() {
    return (
      <div>
        <AuthGuard />
        <span>"Player"</span>
      </div>
    )
  }
}

export default Player;