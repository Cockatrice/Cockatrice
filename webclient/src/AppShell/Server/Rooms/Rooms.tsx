// eslint-disable-next-line
import React from 'react';
import * as _ from 'lodash';

import { RoomsService } from 'AppShell/common/services';

import './Rooms.css';

const Rooms = ({ rooms }) => (
	<div className="rooms">
    <div className="rooms-header">
      <div className="rooms-header__label name">Name</div>
      <div className="rooms-header__label description">Description</div>
      <div className="rooms-header__label permissions">Permissions</div>
      <div className="rooms-header__label players">Players</div>
      <div className="rooms-header__label games">Games</div>
      <div className="rooms-header__label button"></div>
    </div>
		{
			_.map(rooms, ({ description, gameCount, name, permissionlevel, playerCount, roomId }) => (
		    <div className="room" key={roomId}>
		      <div className="room__detail name">{name}</div>
		      <div className="room__detail description">{description}</div>
		      <div className="room__detail permissions">{permissionlevel}</div>
		      <div className="room__detail players">{playerCount}</div>
		      <div className="room__detail games">{gameCount}</div>
		      <div className="room__detail button">
		        <button onClick={() => RoomsService.joinRoom(roomId)}>Join</button>
		      </div>
		    </div>
		  ))
		}
	</div>
);

export default Rooms;
