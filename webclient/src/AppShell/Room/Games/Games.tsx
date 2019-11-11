// eslint-disable-next-line
import React from 'react';
import * as _ from 'lodash';

// import { RoomsService } from 'AppShell/common/services';

import User from 'AppShell/common/components/User/User';

import './Games.css';

const Games = ({ games, gameTypesMap }) => (
  <div className="games">
    <div className="games-header">
      <div className="games-header__label age">Age</div>
      <div className="games-header__label description">Description</div>
      <div className="games-header__label creator">Creator</div>
      <div className="games-header__label type">Type</div>
      <div className="games-header__label restrictions">Restrictions</div>
      <div className="games-header__label players">Players</div>
      <div className="games-header__label spectators">Spectators</div>
      {/*<div className="games-header__label button"></div>*/}
    </div>
    {
      _.map(games, ({ description, gameId, gameTypes, creatorInfo, maxPlayers, playerCount, spectatorsCount }) => (
        <div className="game" key={gameId}>
          <div className="game__detail age">{gameId}</div>
          <div className="game__detail description">{description}</div>
          <div className="game__detail creator"><User user={ creatorInfo } /></div>
          <div className="game__detail type">{gameTypesMap[gameTypes[0]]}</div>
          <div className="game__detail restrictions">?</div>
          <div className="game__detail players">{`${playerCount}/${maxPlayers}`}</div>
          <div className="game__detail spectators">{spectatorsCount}</div>
          {/*<div className="game__detail button">
            <button onClick={() => {}}>Join</button>
          </div>*/}
        </div>
      ))
    }
  </div>
);

export default Games;

