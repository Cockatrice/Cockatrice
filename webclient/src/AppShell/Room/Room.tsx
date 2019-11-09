// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { withRouter, RouteComponentProps } from "react-router-dom";

import { RoomsStateMessages, RoomsStateRooms, Selectors } from 'store/rooms';

import Messages from './Messages/Messages';

import './Room.css';

// @TODO figure out how to properly type this component
// Component<RouteComponentProps<???, ???, ???>>
class Room extends Component<any> {
	render() {
		const { roomId } = this.props.match.params;
		const messages = this.props.messages[roomId];

		return (
			<div>
				<Messages messages={messages} />
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