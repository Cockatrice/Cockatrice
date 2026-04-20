import { SortUtil, RoomsDispatch, RoomsSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

export interface Games {
  sortBy: { field: string; order: string };
  games: any[];
  handleSort: (sortByField: string) => void;
}

export function useGames(roomId: number): Games {
  const sortBy = useAppSelector((state) => RoomsSelectors.getSortGamesBy(state));
  const sortedGames = useAppSelector((state) => RoomsSelectors.getSortedRoomGames(state, roomId));

  const handleSort = (sortByField: string) => {
    const { field, order } = SortUtil.toggleSortBy(sortByField as App.GameSortField, sortBy);
    RoomsDispatch.sortGames(roomId, field, order);
  };

  const isUnavailableGame = ({ started, maxPlayers, playerCount }) =>
    !started && playerCount < maxPlayers;

  const isPasswordProtectedGame = ({ withPassword }) => !withPassword;

  const isBuddiesOnlyGame = ({ onlyBuddies }) => !onlyBuddies;

  const games = sortedGames.filter((game) => (
    isUnavailableGame(game.info) &&
    isPasswordProtectedGame(game.info) &&
    isBuddiesOnlyGame(game.info)
  ));

  return { sortBy, games, handleSort };
}
