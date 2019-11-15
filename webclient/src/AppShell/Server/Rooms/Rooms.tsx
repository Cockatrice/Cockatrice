// eslint-disable-next-line
import React from 'react';
import * as _ from 'lodash';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';

import { RoomsService } from 'AppShell/common/services';

import './Rooms.css';

const Rooms = ({ rooms }) => (
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
              <button onClick={() => RoomsService.joinRoom(roomId)}>Join</button>
            </TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  </div>
);

export default Rooms;
