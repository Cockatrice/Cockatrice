// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { withRouter, RouteComponentProps } from "react-router-dom";

import { RoomsStateMessages, RoomsStateRooms, Selectors } from 'store/rooms';

import { RoomsService } from 'AppShell/common/services';
import SayMessage from 'AppShell/common/components/SayMessage/SayMessage';

import Messages from './Messages/Messages';

import './Room.css';

// @TODO figure out how to properly type this component
// Component<RouteComponentProps<???, ???, ???>>
class Room extends Component<any> {
	constructor(props) {
		super(props);

		this.handleSubmit = this.handleSubmit.bind(this);
	}

	handleSubmit({ message }) {
		if (message) {
			const { roomId } = this.props.match.params;
			RoomsService.roomSay(roomId, message);
		}
	}

	render() {
		const { roomId } = this.props.match.params;
		const messages = this.props.messages[roomId];

		return (
			<div className="room-view">
				<div className="room-view__main">
					<div className="room-view__main-games">
						<div className="room-view__main-games__content overflow-scroll">
							GAMES PANEL
						</div>						
					</div>
					<div className="room-view__main-messages">
						<div className="room-view__main-messages__content overflow-scroll">
							<Messages messages={messages} />
						</div>
						<div className="room-view__main-messages__sayMessage">
							<SayMessage onSubmit={this.handleSubmit} />
						</div>
					</div>
				</div>
				<div className="room-view__sidebar">
					USERS PANEL
				</div>


			</div>
		);
	}
}

interface RoomProps {
	messages: RoomsStateMessages;
	rooms: RoomsStateRooms;
}

const mapStateToProps = state => ({
	messages: Selectors.getMessages(state),
	rooms: Selectors.getRooms(state),
});

export default withRouter(connect(mapStateToProps)(Room));