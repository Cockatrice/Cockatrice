import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from "react-router-dom";

import { ServerService } from 'common/services/data';
import { RouteEnum } from 'common/types';
import {
	Selectors,
	ServerConnectParams,
	ServerStateStatus
} from 'store/server';

import './Header.css';
import logo from './logo.png';

class Header extends Component<HeaderProps, HeaderState> {
	constructor(props) {
		super(props);

		this.connect = this.connect.bind(this);
	}

	componentWillMount() {
		console.log(this);
	}

	connect() {
		const options: ServerConnectParams = {
			host: 'server.cockatrice.us',
			port: '4748',
			user: '',
			pass: '',
		};

		ServerService.connectServer(options);
	}

	render() {
		return (
			<header className="Header">
				<Link to={RouteEnum.MAIN}>
					<img src={logo} className="Header__logo" alt="logo" />
				</Link>
				<div className="Header-content">
					<div className="Header-connect">
						<button onClick={this.connect}>Connect</button>
						{<span className="Header-connect__status">{this.props.status.description}</span>}
					</div>
					{/*<div className="Header-serverDetails">
						<div>Server: server.cockatrice.us</div>
						<div>Room: Magic - Welcome to Rooster Ranges</div>
					</div>*/}
					<nav className="Header-nav">
						<ul className="Header-nav__items">
							<li className="Header-nav__item">
								<Link to={RouteEnum.DECKS}>
									Decks
								</Link>
							</li>
							<strong>|</strong>
							<li className="Header-nav__item">
								<Link to={RouteEnum.ACCOUNT}>
									<div className="Header-account">
										<span className="Header-account__name">
											Seavor
										</span>
										<span className="Header-account__indicator"></span>
									</div>
								</Link>
							</li>
						</ul>
					</nav>
				</div>
			</header>
		)
	}
}

export interface HeaderState {
	status: ServerStateStatus;
}

export interface HeaderProps extends HeaderState {

}

const mapStateToProps = state => ({
	status: Selectors.getStatus(state)
});

export default connect(mapStateToProps)(Header);
