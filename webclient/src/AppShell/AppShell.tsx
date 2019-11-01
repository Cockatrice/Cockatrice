// eslint-disable-next-line
import React from 'react';
import { Provider } from 'react-redux';
import { MemoryRouter as Router } from "react-router-dom";

import './AppShell.css';
import Routes from './AppShellRoutes';
import Header from './Header/Header';

import store from '../store/store';

const AppShell = () => {
  return (
  	<Provider store={store}>
	    <div className="AppShell">
	      <Router>
	        <Header />
	        <Routes />
	      </Router>
	    </div>
	</Provider>
  );
}

export default AppShell;
