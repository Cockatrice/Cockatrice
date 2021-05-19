// eslint-disable-next-line
import React, { Component } from "react";

import { AuthGuard } from "components";

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