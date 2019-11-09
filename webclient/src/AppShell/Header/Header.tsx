import React, { Component } from 'react';
import { connect } from 'react-redux';
import { NavLink } from "react-router-dom";
import * as _ from 'lodash';

import {
	Selectors as RoomsSelectors,
	Room
} from 'store/rooms';

import {
	Selectors as ServerSelectors,
	ServerStateUser
} from 'store/server';

import { AuthenticationService } from 'AppShell/common/services';
import { RouteEnum } from 'AppShell/common/types';

import './Header.css';
import logo from './logo.png';

class Header extends Component<HeaderProps> {
	render() {
		return (
			<div>
				<header className="Header">
					<NavLink to={RouteEnum.SERVER}>
						<img src={logo} className="Header__logo" alt="logo" />
					</NavLink>
					<div className="Header-content">{
						AuthenticationService.isConnected(this.props.state) && (
							<nav className="Header-nav">
								<ul className="Header-nav__items">
									<li className="Header-nav__item">
										<NavLink to={RouteEnum.SERVER}>
											Server ({this.props.server})
										</NavLink>
									</li>
									<strong>|</strong>
									<NavLink to={RouteEnum.ACCOUNT}>
										<div className="Header-account">
											<span className="Header-account__name">
												{this.props.user.name}
											</span>
											<span className="Header-account__indicator"></span>
										</div>
									</NavLink>
								</ul>
							</nav>
						)
					}</div>
				</header>
				<div className="temp-subnav">
					{
						this.props.joinedRooms.length > 0 && (
							<Rooms rooms={this.props.joinedRooms} />
						)
					}
					<div className="temp-subnav__games">
					</div>
					<div className="temp-subnav__chats">
					</div>
				</div>
			</div>
		)
	}
}

const Rooms = props => (
	<div className="temp-subnav__rooms">
		<span>Rooms: </span>
		{
			_.reduce(props.rooms, (rooms, { name, roomId}) => {
				rooms.push(
					<NavLink to={`${RouteEnum.ROOM}/${roomId}`} className="temp-chip" key={roomId}>
						{name}
					</NavLink>
				);
				return rooms;
			}, [])
		}
	</div>
)

interface HeaderProps {
	state: number;
	server: string;
	user: ServerStateUser;
	joinedRooms: Room[];
}

const mapStateToProps = state => ({
	state: ServerSelectors.getState(state),
	server: ServerSelectors.getName(state),
	user: ServerSelectors.getUser(state),
	joinedRooms: RoomsSelectors.getJoinedRooms(state)
});

export default connect(mapStateToProps)(Header);
