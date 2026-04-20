import { useCallback, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import {
  GameSelectors,
  GameTypes,
  RoomsDispatch,
  RoomsSelectors,
  ServerSelectors,
  useAppSelector,
  type GameFilters,
} from '@app/store';
import { useReduxEffect, useWebClient } from '@app/hooks';
import { App, type Enriched } from '@app/types';
import { AlertDialog, CreateGameDialog, FilterGamesDialog, PromptDialog } from '@app/dialogs';

import OpenGames from '../OpenGames';
import GameSelectorToolbar from './GameSelectorToolbar';

import './GameSelector.css';

interface GameSelectorProps {
  room: Enriched.Room;
}

interface PendingPasswordJoin {
  gameId: number;
  asSpectator: boolean;
  asJudge: boolean;
}

const GameSelector = ({ room }: GameSelectorProps) => {
  const roomId = room.info.roomId;
  const webClient = useWebClient();
  const navigate = useNavigate();

  const selectedGameId = useAppSelector((state) => RoomsSelectors.getSelectedGameId(state, roomId));
  const selectedGame = useAppSelector((state) =>
    selectedGameId != null ? RoomsSelectors.getRoomGames(state, roomId)[selectedGameId] : undefined,
  );
  const counts = useAppSelector((state) => RoomsSelectors.getRoomGameCounts(state, roomId));
  const isFilterActive = useAppSelector((state) => RoomsSelectors.isGameFilterActive(state, roomId));
  const filters = useAppSelector((state) => RoomsSelectors.getGameFilters(state, roomId));
  const isJudgeUser = useAppSelector(ServerSelectors.getIsUserJudge);
  const joinPending = useAppSelector(RoomsSelectors.getJoinGamePending);
  const joinError = useAppSelector(RoomsSelectors.getJoinGameError);
  const activeGameIds = useAppSelector(GameSelectors.getActiveGameIds);

  // Mirrors Server.tsx's JOIN_ROOM → navigate(ROOM) pattern: when Event_GameJoined
  // lands, we're actually in the game — route to /game.
  useReduxEffect(() => {
    navigate(App.RouteEnum.GAME);
  }, GameTypes.GAME_JOINED, [navigate]);

  const [createOpen, setCreateOpen] = useState(false);
  const [filterOpen, setFilterOpen] = useState(false);
  const [pendingPasswordJoin, setPendingPasswordJoin] = useState<PendingPasswordJoin | null>(null);

  const sendJoin = useCallback(
    (gameId: number, asSpectator: boolean, asJudge: boolean, password: string) => {
      // Mirrors Rooms.tsx short-circuit: if we already have a live game entry
      // (Event_GameJoined has populated games.games[gameId]), skip the duplicate
      // JoinGame — the server would reject it with RespContextError — and go
      // straight to the game view.
      if (activeGameIds.includes(gameId)) {
        navigate(App.RouteEnum.GAME);
        return;
      }
      const params: App.JoinGameParams = {
        gameId,
        password,
        spectator: asSpectator,
        overrideRestrictions: false,
        joinAsJudge: asJudge,
      };
      webClient.request.rooms.joinGame(roomId, params);
    },
    [activeGameIds, navigate, roomId, webClient],
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
        setPendingPasswordJoin({ gameId: info.gameId, asSpectator: effectiveSpectator, asJudge });
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

  const canJoin =
    Boolean(selectedGame && selectedGame.info.playerCount < selectedGame.info.maxPlayers) && !joinPending;
  const canSpectate = Boolean(selectedGame && selectedGame.info.spectatorsAllowed) && !joinPending;

  const handleCreateSubmit = (params: App.CreateGameParams) => {
    webClient.request.rooms.createGame(roomId, params);
    setCreateOpen(false);
  };

  const handleFilterSubmit = (next: GameFilters) => {
    RoomsDispatch.setGameFilters(roomId, next);
    setFilterOpen(false);
  };

  const handlePasswordSubmit = (password: string) => {
    if (!pendingPasswordJoin) {
      return;
    }
    sendJoin(pendingPasswordJoin.gameId, pendingPasswordJoin.asSpectator, pendingPasswordJoin.asJudge, password);
    setPendingPasswordJoin(null);
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
        isOpen={pendingPasswordJoin !== null}
        title="Password required"
        label="Password"
        submitLabel="Join"
        onSubmit={handlePasswordSubmit}
        onCancel={() => setPendingPasswordJoin(null)}
      />
      <AlertDialog
        isOpen={joinError !== null}
        title="Error"
        message={joinError?.message ?? ''}
        onDismiss={() => RoomsDispatch.clearJoinGameError()}
      />
    </Paper>
  );
};

export default GameSelector;
