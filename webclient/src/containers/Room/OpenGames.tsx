import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import TableSortLabel from '@mui/material/TableSortLabel';
import Tooltip from '@mui/material/Tooltip';

import { UserDisplay } from '@app/components';
import { Data, Enriched } from '@app/types';

import { useOpenGames } from './useOpenGames';

import './OpenGames.css';

interface OpenGamesProps {
  room: { info: { roomId: number } };
  onActivateGame?: (gameId: number) => void;
}

function formatRestrictions(info: Data.ServerInfo_Game): string {
  const parts: string[] = [];
  if (info.withPassword) {
    parts.push('password');
  }
  if (info.onlyBuddies) {
    parts.push('buddies only');
  }
  if (info.onlyRegistered) {
    parts.push('reg. users only');
  }
  if (info.shareDecklistsOnLoad) {
    parts.push('open decklists');
  }
  return parts.join(', ');
}

function formatSpectators(info: Data.ServerInfo_Game): string {
  if (!info.spectatorsAllowed) {
    return 'not allowed';
  }
  const flags: string[] = [];
  if (info.spectatorsCanChat) {
    flags.push('can chat');
  }
  if (info.spectatorsOmniscient) {
    flags.push('see hands');
  }
  if (flags.length === 0) {
    return String(info.spectatorsCount);
  }
  return `${info.spectatorsCount} (${flags.join(' & ')})`;
}

const OpenGames = ({ room, onActivateGame }: OpenGamesProps) => {
  const roomId = room.info.roomId;
  const { sortBy, games, selectedGameId, handleSort, handleSelect, handleActivate } = useOpenGames({
    roomId,
    onActivateGame,
  });

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
          {games.map((game: Enriched.Game) => {
            const { info, gameType } = game;
            const { description, gameId, creatorInfo, maxPlayers, playerCount, startTime } = info;
            const isSelected = gameId === selectedGameId;
            const restrictions = formatRestrictions(info);
            const spectators = formatSpectators(info);
            return (
              <TableRow
                key={gameId}
                hover
                selected={isSelected}
                onClick={() => handleSelect(gameId)}
                onDoubleClick={() => handleActivate(gameId)}
                className={isSelected ? 'games__row games__row--selected' : 'games__row'}
              >
                <TableCell className="games-header__cell single-line-ellipsis">{startTime}</TableCell>
                <TableCell className="games-header__cell">
                  <Tooltip title={description} placement="bottom-start" enterDelay={500}>
                    <div className="single-line-ellipsis">
                      {description}
                    </div>
                  </Tooltip>
                </TableCell>
                <TableCell className="games-header__cell">
                  {creatorInfo ? <UserDisplay user={creatorInfo} /> : null}
                </TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{gameType}</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{restrictions}</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{`${playerCount}/${maxPlayers}`}</TableCell>
                <TableCell className="games-header__cell single-line-ellipsis">{spectators}</TableCell>
              </TableRow>
            );
          })}
        </TableBody>
      </Table>
    </div>
  );
};

export default OpenGames;
