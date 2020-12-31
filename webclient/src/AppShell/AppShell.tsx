// eslint-disable-next-line
import React, { Component } from "react";
import { Provider } from "react-redux";
import { MemoryRouter as Router } from "react-router-dom";
import CssBaseline from "@material-ui/core/CssBaseline";

import { store } from "store";

import "./AppShell.css";
import Routes from "./AppShellRoutes";
import Header from "./Header/Header";


class AppShell extends Component {
  componentDidMount() {
    // @TODO (1)
    window.onbeforeunload = () => true;
  }

  handleContextMenu(event) {
    event.preventDefault();
  }

  render() {
    return (
      <Provider store={store}>
        <CssBaseline />
        <div className="AppShell" onContextMenu={this.handleContextMenu}>
          <Router>
            <Header />
            <Routes />
          </Router>
        </div>
      </Provider>
    );
  }
}

export default AppShell;
