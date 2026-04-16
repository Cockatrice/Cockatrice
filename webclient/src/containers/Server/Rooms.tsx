// eslint-disable-next-line
import React from "react";
import { generatePath, useNavigate } from 'react-router-dom';

import Button from '@mui/material/Button';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';

import { useWebClient } from '@app/hooks';
import { App } from '@app/types';

import './Rooms.css';

const Rooms = ({ rooms, joinedRooms }) => {
  const navigate = useNavigate();
  const webClient = useWebClient();

  function onClick(roomId) {
    if (joinedRooms.find(room => room.info.roomId === roomId)) {
      navigate(generatePath(App.RouteEnum.ROOM, { roomId }));
    } else {
      webClient.request.rooms.joinRoom(roomId);
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
          { Object.values(rooms).map((room) => {
            const { description, gameCount, name, permissionlevel, playerCount, roomId } = room.info;
            return (
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
            );
          })}
        </TableBody>
      </Table>
    </div>
  );
};

export default Rooms;
