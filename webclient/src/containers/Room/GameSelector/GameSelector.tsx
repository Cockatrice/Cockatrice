import React, { useCallback, useState } from 'react';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import { RoomsDispatch, RoomsSelectors, ServerSelectors, useAppSelector } from '@app/store';
import { useWebClient } from '@app/hooks';
import type { App, Enriched } from '@app/types';
import { CreateGameDialog, FilterGamesDialog, PromptDialog } from '@app/dialogs';

import OpenGames from '../OpenGames';
import GameSelectorToolbar from './GameSelectorToolbar';

import './GameSelector.css';

interface GameSelectorProps {
  room: Enriched.Room;
}

interface PendingJoin {
  gameId: number;
  asSpectator: boolean;
  asJudge: boolean;
}

const GameSelector = ({ room }: GameSelectorProps) => {
  const roomId = room.info.roomId;
  const webClient = useWebClient();

  const selectedGameId = useAppSelector((state) => RoomsSelectors.getSelectedGameId(state, roomId));
  const selectedGame = useAppSelector((state) =>
    selectedGameId != null ? RoomsSelectors.getRoomGames(state, roomId)[selectedGameId] : undefined,
  );
  const counts = useAppSelector((state) => RoomsSelectors.getRoomGameCounts(state, roomId));
  const isFilterActive = useAppSelector((state) => RoomsSelectors.isGameFilterActive(state, roomId));
  const filters = useAppSelector((state) => RoomsSelectors.getGameFilters(state, roomId));
  const isJudgeUser = useAppSelector(ServerSelectors.getIsUserJudge);

  const [createOpen, setCreateOpen] = useState(false);
  const [filterOpen, setFilterOpen] = useState(false);
  const [pendingJoin, setPendingJoin] = useState<PendingJoin | null>(null);

  const sendJoin = useCallback(
    (gameId: number, asSpectator: boolean, asJudge: boolean, password: string) => {
      const params: App.JoinGameParams = {
        gameId,
        password,
        spectator: asSpectator,
        overrideRestrictions: false,
        joinAsJudge: asJudge,
      };
      webClient.request.rooms.joinGame(roomId, params);
    },
    [roomId, webClient],
  );

  const beginJoin = useCallback(
    (asSpectator: boolean, asJudge: boolean) => {
      const game = selectedGame;
      if (!game) {
        return;
      }
      const info = game.info;
      const effectiveSpectator =
        asSpectator || info.playerCount >= info.maxPlayers;
      const needsPassword =
        info.withPassword && !(effectiveSpectator && !info.spectatorsNeedPassword);
      if (needsPassword) {
        setPendingJoin({ gameId: info.gameId, asSpectator: effectiveSpectator, asJudge });
        return;
      }
      sendJoin(info.gameId, effectiveSpectator, asJudge, '');
    },
    [selectedGame, sendJoin],
  );

  const handleActivate = useCallback(
    (_gameId: number) => {
      beginJoin(false, false);
    },
    [beginJoin],
  );

  const canJoin = Boolean(selectedGame && selectedGame.info.playerCount < selectedGame.info.maxPlayers);
  const canSpectate = Boolean(selectedGame && selectedGame.info.spectatorsAllowed);

  const handleCreateSubmit = (params: App.CreateGameParams) => {
    webClient.request.rooms.createGame(roomId, params);
    setCreateOpen(false);
  };

  const handleFilterSubmit = (next) => {
    RoomsDispatch.setGameFilters(roomId, next);
    setFilterOpen(false);
  };

  const handlePasswordSubmit = (password: string) => {
    if (!pendingJoin) {
      return;
    }
    sendJoin(pendingJoin.gameId, pendingJoin.asSpectator, pendingJoin.asJudge, password);
    setPendingJoin(null);
  };

  return (
    <Paper className="game-selector overflow-scroll">
      <Typography className="game-selector__title" variant="subtitle2">
        Games shown: {counts.visible} / {counts.total}
      </Typography>
      <div className="game-selector__games">
        <OpenGames room={room} onActivateGame={handleActivate} />
      </div>
      <GameSelectorToolbar
        isFilterActive={isFilterActive}
        canCreate={true}
        canJoin={canJoin}
        canSpectate={canSpectate}
        isJudgeUser={isJudgeUser}
        onFilter={() => setFilterOpen(true)}
        onClearFilter={() => RoomsDispatch.clearGameFilters(roomId)}
        onCreate={() => setCreateOpen(true)}
        onJoin={() => beginJoin(false, false)}
        onSpectate={() => beginJoin(true, false)}
        onJoinAsJudge={() => beginJoin(false, true)}
        onSpectateAsJudge={() => beginJoin(true, true)}
      />

      <CreateGameDialog
        isOpen={createOpen}
        gametypeMap={room.gametypeMap}
        onCancel={() => setCreateOpen(false)}
        onSubmit={handleCreateSubmit}
      />
      <FilterGamesDialog
        isOpen={filterOpen}
        initialFilters={filters}
        gametypeMap={room.gametypeMap}
        onCancel={() => setFilterOpen(false)}
        onSubmit={handleFilterSubmit}
      />
      <PromptDialog
        isOpen={pendingJoin !== null}
        title="Password required"
        label="Password"
        submitLabel="Join"
        onSubmit={handlePasswordSubmit}
        onCancel={() => setPendingJoin(null)}
      />
    </Paper>
  );
};

export default GameSelector;
