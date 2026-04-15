// eslint-disable-next-line
import React from "react";
import * as _ from 'lodash';

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

import './Games.css';

// @TODO run interval to update timeSinceCreated
interface GamesProps {
  room: any;
}

const Games = ({ room }: GamesProps) => {
  const sortBy = useAppSelector(state => RoomsSelectors.getSortGamesBy(state));

  const headerCells = [
    { label: 'Age', field: 'startTime' },
    { label: 'Description', field: 'description' },
    { label: 'Creator', field: 'creatorInfo.name' },
    { label: 'Type', field: 'gameType' },
    { label: 'Restrictions' },
    { label: 'Players' },
    { label: 'Spectators', field: 'spectatorsCount' },
  ];

  const handleSort = (sortByField) => {
    const { roomId } = room;
    const { field, order } = SortUtil.toggleSortBy(sortByField, sortBy);
    RoomsDispatch.sortGames(roomId, field, order);
  };

  const isUnavailableGame = ({ started, maxPlayers, playerCount }) =>
    !started && playerCount < maxPlayers;

  const isPasswordProtectedGame = ({ withPassword }) => !withPassword;

  const isBuddiesOnlyGame = ({ onlyBuddies }) => !onlyBuddies;

  const games = room.gameList.filter(game => (
    isUnavailableGame(game) &&
    isPasswordProtectedGame(game) &&
    isBuddiesOnlyGame(game)
  ));

  return (
    <div className="games">
      <Table size="small">
        <TableHead>
          <TableRow>
            { _.map(headerCells, ({ label, field }) => {
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
          { _.map(games, ({ description, gameId, gameType, creatorInfo, maxPlayers, playerCount, spectatorsCount, startTime }) => (
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
          ))}
        </TableBody>
      </Table>
    </div>
  );
};

export default Games;
