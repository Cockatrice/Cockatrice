import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from "react-router-dom";

import {
	Selectors as RoomsSelectors,
	RoomsStateRoom
} from 'store/rooms';
import {
	Selectors as ServerSelectors,
	ServerConnectParams,
	ServerStateUser
} from 'store/server';
import webClient, { WebClient } from 'WebClient/WebClient';

import { RouteEnum } from '../common/types';

import { Navigation, SignIn } from './components';

import './Header.css';
import logo from './logo.png';

class Header extends Component<HeaderProps> {
	webClient: WebClient = webClient;

	constructor(props) {
		super(props);

		this.connect = this.connect.bind(this);
	}

	connect() {
		const options: ServerConnectParams = {
			host: 'server.cockatrice.us',
			port: '4748',
			user: '',
			pass: '',
		};

		this.webClient.services.server.connectServer(options);
	}

	render() {
		return (
			<header className="Header">
				<Link to={RouteEnum.SERVER}>
					<img src={logo} className="Header__logo" alt="logo" />
				</Link>
				<div className="Header-content">{
					this.props.user
						? <Navigation user={this.props.user} />
						: <SignIn connect={this.connect} />
				}</div>
			</header>
		)
	}
}

export interface HeaderProps {
	description: string;
	user: ServerStateUser;
	room: RoomsStateRoom;
}

const mapStateToProps = state => ({
	description: ServerSelectors.getDescription(state),
	user: ServerSelectors.getUser(state),
	room: RoomsSelectors.getRoom(state, RoomsSelectors.getActive(state))
});

export default connect(mapStateToProps)(Header);
