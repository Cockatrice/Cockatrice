// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { withRouter /*, RouteComponentProps */ } from "react-router-dom";
import { ServerSelectors } from "store";

import { AuthGuard } from "components/index";
import { SessionService } from "websocket";

import "./Decks.css";

class Decks extends Component<any> {
  constructor(props) {
    super(props)

    this.handleListDecks = this.handleListDecks.bind(this)
  }

  handleListDecks () {
    SessionService.listDecks()
  }

  render() {
    console.log(this.props.decks)

    return (
      <div>
        <AuthGuard />
        <span>"Decks"</span>
        <button onClick={this.handleListDecks}>
          LOAD
        </button>
      </div>
    )
  }
}

interface DecksProps {
  decks: any[];
}

const mapStateToProps = state => ({
  decks: ServerSelectors.getDecks(state),
});

export default withRouter(connect(mapStateToProps)(Decks));
