import { useMemo } from 'react';
import { generatePath, useNavigate } from 'react-router-dom';

import Button from '@mui/material/Button';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';

import { useWebClient } from '@app/hooks';
import { App, Enriched } from '@app/types';

import './Rooms.css';

interface RoomsProps {
  rooms: Record<number, Enriched.Room>;
  joinedRooms: Enriched.Room[];
}

const Rooms = ({ rooms, joinedRooms }: RoomsProps) => {
  const navigate = useNavigate();
  const webClient = useWebClient();

  const joinedRoomIds = useMemo(
    () => new Set(joinedRooms.map((room) => room.info.roomId)),
    [joinedRooms],
  );

  const onClick = (roomId: number) => {
    if (joinedRoomIds.has(roomId)) {
      navigate(generatePath(App.RouteEnum.ROOM, { roomId: String(roomId) }));
    } else {
      webClient.request.rooms.joinRoom(roomId);
    }
  };

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
          {Object.values(rooms).map((room) => {
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
