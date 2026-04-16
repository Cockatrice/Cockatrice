// eslint-disable-next-line
import React from "react";

import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import TableSortLabel from '@mui/material/TableSortLabel';
import Tooltip from '@mui/material/Tooltip';

// import { RoomsService } from "AppShell/common/services";

import { SortUtil, RoomsDispatch, RoomsSelectors } from '@app/store';
import { UserDisplay } from '@app/components';
import { useAppSelector } from '@app/store';

import './OpenGames.css';

// @TODO run interval to update timeSinceCreated
interface OpenGamesProps {
  room: any;
}

const OpenGames = ({ room }: OpenGamesProps) => {
  const roomId = room.info.roomId;
  const sortBy = useAppSelector(state => RoomsSelectors.getSortGamesBy(state));
  const sortedGames = useAppSelector(state => RoomsSelectors.getSortedRoomGames(state, roomId));

  const headerCells = [
    { label: 'Age', field: 'info.startTime' },
    { label: 'Description', field: 'info.description' },
    { label: 'Creator', field: 'info.creatorInfo.name' },
    { label: 'Type', field: 'gameType' },
    { label: 'Restrictions' },
    { label: 'Players' },
    { label: 'Spectators', field: 'info.spectatorsCount' },
  ];

  const handleSort = (sortByField) => {
    const { field, order } = SortUtil.toggleSortBy(sortByField, sortBy);
    RoomsDispatch.sortGames(roomId, field, order);
  };

  const isUnavailableGame = ({ started, maxPlayers, playerCount }) =>
    !started && playerCount < maxPlayers;

  const isPasswordProtectedGame = ({ withPassword }) => !withPassword;

  const isBuddiesOnlyGame = ({ onlyBuddies }) => !onlyBuddies;

  const games = sortedGames.filter(game => (
    isUnavailableGame(game.info) &&
    isPasswordProtectedGame(game.info) &&
    isBuddiesOnlyGame(game.info)
  ));

  return (
    <div className="games">
      <Table size="small">
        <TableHead>
          <TableRow>
            { headerCells.map(({ label, field }) => {
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
          { games.map((game) => {
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
                  <UserDisplay user={ creatorInfo } />
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

export default OpenGames;
