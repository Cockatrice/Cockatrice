// eslint-disable-next-line
import React from "react";
import { generatePath, useNavigate } from 'react-router-dom';
import * as _ from 'lodash';

import Button from '@mui/material/Button';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';


import { RoomsService } from 'api';
import { RouteEnum } from 'types';

import './Rooms.css';

const Rooms = ({ rooms, joinedRooms }) => {
  const navigate = useNavigate();

  function onClick(roomId) {
    if (_.find(joinedRooms, room => room.roomId === roomId)) {
      navigate(generatePath(RouteEnum.ROOM, { roomId }));
    } else {
      RoomsService.joinRoom(roomId);
    }
  }

  return (
    <div className="rooms">
      <Table size="small">
        <TableHead>
          <TableRow>
            <TableCell>Name</TableCell>
            <TableCell>Description</TableCell>
            <TableCell>Permissions</TableCell>
            <TableCell>Players</TableCell>
            <TableCell>Games</TableCell>
            <TableCell></TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          { _.map(rooms, ({ description, gameCount, name, permissionlevel, playerCount, roomId }) => (
            <TableRow key={roomId}>
              <TableCell>{name}</TableCell>
              <TableCell>{description}</TableCell>
              <TableCell>{permissionlevel}</TableCell>
              <TableCell>{playerCount}</TableCell>
              <TableCell>{gameCount}</TableCell>
              <TableCell>
                <Button size="small" color="primary" variant="contained" onClick={() => onClick(roomId)}>
                  Join
                </Button>
              </TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </div>
  );
};

export default Rooms;
