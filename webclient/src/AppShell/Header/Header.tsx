import React, { Component } from 'react';
import { Link } from "react-router-dom";

import './Header.css';
import logo from './logo.png';
import RouteEnum from '../../common/types/RouteEnum';
import WebClient from '../../common/services/WebClient/WebClient';

class Header extends Component<HeaderProps, HeaderState> {
	constructor(props: HeaderProps) {
		super(props);

		this.state = {
			status: WebClient.getStatusEnumLabel(WebClient.StatusEnum.DISCONNECTED)
		};
		this.connect = this.connect.bind(this);
		this.handleStatusCallback = this.handleStatusCallback.bind(this);
	}

	connect() {
		WebClient.getInstance().connect({
			host: 'chickatrice.net',
			port: '4748',
			user: '',
			pass: '',
	        statusCallback: this.handleStatusCallback,
	        connectionClosedCallback: (id, desc) => console.log(`connectionClosedCallback [${id}] : `, desc),
	        serverMessageCallback: (message) => console.log(`serverMessageCallback : `, message),
	        serverIdentificationCallback: (data) => console.log(`serverIdentificationCallback : `, data),
	        serverShutdownCallback: (data) => console.log(`serverShutdownCallback : `, data),
	        notifyUserCallback: (data) => console.log(`notifyUserCallback : `, data),
	        userInfoCallback: (data) => console.log(`userInfoCallback : `, data),
	        listRoomsCallback: (rooms) => console.log(`listRoomsCallback : `, rooms),
	        errorCallback: (id, desc) => console.log(`errorCallback [${id}] : `, desc),
	        joinRoomCallback: (room) => console.log(`joinRoomCallback : `, room),
	        roomMessageCallback: (roomId, message) => console.log(`roomMessageCallback [${roomId}] : `, message),
	        roomJoinCallback: (roomId, message) => console.log(`roomJoinCallback [${roomId}] : `, message),
	        roomLeaveCallback: (roomId, message) => console.log(`roomLeaveCallback [${roomId}] : `, message),
	        roomListGamesCallback: (roomId, message) => console.log(`roomListGamesCallback [${roomId}] : `, message)
		});
	}

	handleStatusCallback(id, desc) {
		this.setState({
			status: desc
		});
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
						{<span className="Header-connect__status">{this.state.status}</span>}
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

export interface HeaderProps {}

export interface HeaderState {
	status: number
}

export default Header;