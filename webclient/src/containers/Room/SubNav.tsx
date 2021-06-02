import React from "react";
import { NavLink, generatePath } from "react-router-dom";
import Chip from "@material-ui/core/Chip";
import * as _ from "lodash";

import { RoomsService } from "api";
import { Room, RouteEnum } from "types";

import "./SubNav.css";

const SubNav = ({ rooms }: SubNavProps) => {
  return (
    <div className="rooms-subnav">
      <span>Rooms: </span>
      {
        _.reduce(rooms, (rooms, { name, roomId}) => {
          rooms.push(
            <NavLink to={generatePath(RouteEnum.ROOM, { roomId })} className="chip" key={roomId}>
              <Chip label={name} color="primary" onDelete={(event) => onLeaveRoom(event, roomId)} />
            </NavLink>
          );
          return rooms;
        }, [])
      }
    </div>
  );

  function onLeaveRoom(event, roomId): void {
    event.preventDefault();
    RoomsService.leaveRoom(roomId);
  };
};

interface SubNavProps {
  rooms: Room[];
}

export default SubNav;