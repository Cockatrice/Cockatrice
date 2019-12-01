// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import * as _ from "lodash";

import Table from "@material-ui/core/Table";
import TableBody from "@material-ui/core/TableBody";
import TableCell from "@material-ui/core/TableCell";
import TableHead from "@material-ui/core/TableHead";
import TableRow from "@material-ui/core/TableRow";
import TableSortLabel from "@material-ui/core/TableSortLabel";
import Tooltip from "@material-ui/core/Tooltip";

// import { RoomsService } from "AppShell/common/services";

import { SortUtil } from "store/common";
import { Dispatch, Selectors } from "store/rooms";
import UserDisplay from "AppShell/common/components/UserDisplay/UserDisplay";

import "./Games.css";

// @TODO run interval to update timeSinceCreated
class Games extends Component<GamesProps> {
  private headerCells = [
    {
      label: "Age",
      field: "startTime"
    },
    {
      label: "Description",
      field: "description"
    },
    {
      label: "Creator",
      field: "creatorInfo.name"
    },
    {
      label: "Type",
      field: "gameType"
    },
    {
      label: "Restrictions",
      // field: "?"
    },
    {
      label: "Players",
      // field: ["maxPlayers", "playerCount"]
    },
    {
      label: "Spectators",
      field: "spectatorsCount"
    },
  ];

  handleSort(sortByField) {
    const { room: { roomId }, sortBy } = this.props;
    const { field, order } = SortUtil.toggleSortBy(sortByField, sortBy);
    Dispatch.sortGames(roomId, field, order);
  }

  private isUnavailableGame({ started, maxPlayers, playerCount }) {
    return !started && playerCount < maxPlayers;
  }

  private isPasswordProtectedGame({ withPassword }) {
    return !withPassword;
  }

  private isBuddiesOnlyGame({ onlyBuddies }) {
    return !onlyBuddies;
  }

  render() {
    const { room, sortBy } = this.props;

    const games = room.gameList.filter(game => (
      this.isUnavailableGame(game) &&
      this.isPasswordProtectedGame(game) &&
      this.isBuddiesOnlyGame(game)
    ));

    return (
      <div className="games">
        <Table size="small">
          <TableHead>
            <TableRow>
              { _.map(this.headerCells, ({ label, field }) => {
                const active = field === sortBy.field;
                const order = sortBy.order.toLowerCase();
                const sortDirection = active ? order : false;

                return (
                  <TableCell sortDirection={sortDirection} key={label}>
                    {!field ? label : (
                      <TableSortLabel
                        active={active}
                        direction={order}
                        onClick={() => this.handleSort(field)}
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
  }
}

interface GamesProps {
  room: any;
  sortBy: any;
}

const mapStateToProps = state => ({
  sortBy: Selectors.getSortGamesBy(state)
});

export default connect(mapStateToProps)(Games);
