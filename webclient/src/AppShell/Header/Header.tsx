import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from "react-router-dom";

import { Selectors, ServerStateUser } from 'store/server';

import { AuthenticationService } from 'AppShell/common/services';
import { RouteEnum } from 'AppShell/common/types';

import './Header.css';
import logo from './logo.png';

class Header extends Component<HeaderProps> {
	render() {
		return (
			<header className="Header">
				<Link to={RouteEnum.SERVER}>
					<img src={logo} className="Header__logo" alt="logo" />
				</Link>
				<div className="Header-content">{
					AuthenticationService.isConnected(this.props.state) && (
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
											{this.props.user.name}
										</span>
										<span className="Header-account__indicator"></span>
									</div>
								</Link>
							</ul>
						</nav>
					)
				}</div>
			</header>
		)
	}
}

export interface HeaderProps {
	state: number;
	user: ServerStateUser;
}

const mapStateToProps = state => ({
	state: Selectors.getState(state),
	user: Selectors.getUser(state)
});

export default connect(mapStateToProps)(Header);
