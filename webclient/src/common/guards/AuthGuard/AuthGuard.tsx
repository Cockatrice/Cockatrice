// eslint-disable-next-line
import React, { Component } from 'react';
import {
	Redirect
} from "react-router-dom";

const authenticated = true;

class AuthGuard extends Component {
	render() {
		if (!authenticated) {
			return <Redirect to={{
              pathname: '/connect',
              state: { from: '/decks' }
            }} />;
		}

		return '';
	}
}

export default AuthGuard;