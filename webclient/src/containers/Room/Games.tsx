import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import TableSortLabel from '@mui/material/TableSortLabel';
import Tooltip from '@mui/material/Tooltip';

import { UserDisplay } from '@app/components';

import { useGames } from './useGames';

import './Games.css';

// @TODO run interval to update timeSinceCreated
interface GamesProps {
  room: any;
}

const Games = ({ room }: GamesProps) => {
  const roomId = room.info.roomId;
  const { sortBy, games, handleSort } = useGames(roomId);

  const headerCells = [
    { label: 'Age', field: 'info.startTime' },
    { label: 'Description', field: 'info.description' },
    { label: 'Creator', field: 'info.creatorInfo.name' },
    { label: 'Type', field: 'gameType' },
    { label: 'Restrictions' },
    { label: 'Players' },
    { label: 'Spectators', field: 'info.spectatorsCount' },
  ];

  return (
    <div className="games">
      <Table size="small">
        <TableHead>
          <TableRow>
            {headerCells.map(({ label, field }) => {
              const active = field === sortBy.field;
              const order = sortBy.order.toLowerCase();
              const sortDirection = active ? (order === 'asc' ? 'asc' : 'desc') : false;

              return (
                <TableCell sortDirection={sortDirection} key={label}>
                  {!field ? label : (
                    <TableSortLabel
                      active={active}
                      direction={order}
                      onClick={() => handleSort(field)}
                    >
                      {label}
                    </TableSortLabel>
                  )}
                </TableCell>
              );
            })}
          </TableRow>
        </TableHead>
        <TableBody>
          {games.map((game) => {
            const { info, gameType } = game;
            const { description, gameId, creatorInfo, maxPlayers, playerCount, spectatorsCount, startTime } = info;
            return (
              <TableRow key={gameId}>
                <TableCell className="games-header__cell single-line-ellipsis">{startTime}</TableCell>
                <TableCell className="games-header__cell">
                  <Tooltip title={description} placement="bottom-start" enterDelay={500}>
                    <div className="single-line-ellipsis">
                      {description}
                    </div>
                  </Tooltip>
                </TableCell>
                <TableCell className="games-header__cell">
                  <UserDisplay user={creatorInfo} />
                </TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{gameType}</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">?</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{`${playerCount}/${maxPlayers}`}</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{spectatorsCount}</TableCell>
              </TableRow>
            );
          })}
        </TableBody>
      </Table>
    </div>
  );
};

export default Games;
