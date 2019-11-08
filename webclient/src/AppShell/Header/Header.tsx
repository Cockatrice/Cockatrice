import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from "react-router-dom";

import {
	Selectors,
	ServerConnectParams,
	ServerStateUser
} from 'store/server';
import webClient, { WebClient } from 'WebClient/WebClient';

import { RouteEnum } from '../common/types';

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
		let template;

		if (this.props.user) {
			template = (<Navigation user={this.props.user} />);
		} else {
			template = (<SignIn connect={this.connect} />);
		}

		return (
			<header className="Header">
				<Link to={RouteEnum.SERVER}>
					<img src={logo} className="Header__logo" alt="logo" />
				</Link>
				<div className="Header-content">
					{template}
				</div>
			</header>
		)
	}
}

function Navigation(props) {
	return (
		<nav className="Header-nav">
			<ul className="Header-nav__items">
				<li className="Header-nav__item">
					<Link to={RouteEnum.DECKS}>
						Decks
					</Link>
				</li>
				<strong>|</strong>
				<Link to={RouteEnum.ACCOUNT}>
					<div className="Header-account">
						<span className="Header-account__name">
							{props.user.name}
						</span>
						<span className="Header-account__indicator"></span>
					</div>
				</Link>
			</ul>
		</nav>
	);
}

function SignIn(props) {
	return (
		<div className="Header-connect">
			<button onClick={props.connect}>Connect</button>
		</div>
	);
}

export interface HeaderProps {
	description: string;
	user: ServerStateUser;
}

const mapStateToProps = state => ({
	description: Selectors.getDescription(state),
	user: Selectors.getUser(state)
});

export default connect(mapStateToProps)(Header);
