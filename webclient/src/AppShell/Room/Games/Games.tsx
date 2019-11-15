// eslint-disable-next-line
import React from 'react';
import * as _ from 'lodash';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';

// import { RoomsService } from 'AppShell/common/services';

import UserDisplay from 'AppShell/common/components/UserDisplay/UserDisplay';

import './Games.css';

const Games = ({ games, gameTypesMap }) => (
  <div className="games">
    <Table size="small">
      <TableHead>
        <TableRow>
          <TableCell>Age</TableCell>
          <TableCell>Description</TableCell>
          <TableCell>Creator</TableCell>
          <TableCell>Type</TableCell>
          <TableCell>Restrictions</TableCell>
          <TableCell>Players</TableCell>
          <TableCell>Spectators</TableCell>
        </TableRow>
      </TableHead>
      <TableBody>
        { _.map(games, ({ description, gameId, gameTypes, creatorInfo, maxPlayers, playerCount, spectatorsCount }) => (
          <TableRow key={gameId}>
            <TableCell>{gameId}</TableCell>
            <TableCell>{description}</TableCell>
            <TableCell>
              <UserDisplay user={ creatorInfo } />
            </TableCell>
            <TableCell>{gameTypesMap[gameTypes[0]]}</TableCell>
            <TableCell>?</TableCell>
            <TableCell>{`${playerCount}/${maxPlayers}`}</TableCell>
            <TableCell>{spectatorsCount}</TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  </div>
);

export default Games;

