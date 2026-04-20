import { SortUtil, RoomsDispatch, RoomsSelectors, useAppSelector } from '@app/store';
import { App, Enriched } from '@app/types';

export interface OpenGames {
  sortBy: { field: string; order: string };
  games: Enriched.Game[];
  selectedGameId: number | undefined;
  handleSort: (sortByField: string) => void;
  handleSelect: (gameId: number) => void;
  handleActivate: (gameId: number) => void;
}

export interface UseOpenGamesArgs {
  roomId: number;
  onActivateGame?: (gameId: number) => void;
}

export function useOpenGames({ roomId, onActivateGame }: UseOpenGamesArgs): OpenGames {
  const sortBy = useAppSelector((state) => RoomsSelectors.getSortGamesBy(state));
  const games = useAppSelector((state) => RoomsSelectors.getFilteredRoomGames(state, roomId));
  const selectedGameId = useAppSelector((state) => RoomsSelectors.getSelectedGameId(state, roomId));

  const handleSort = (sortByField: string) => {
    const { field, order } = SortUtil.toggleSortBy(sortByField as App.GameSortField, sortBy);
    RoomsDispatch.sortGames(roomId, field, order);
  };

  const handleSelect = (gameId: number) => {
    RoomsDispatch.selectGame(roomId, gameId);
  };

  const handleActivate = (gameId: number) => {
    RoomsDispatch.selectGame(roomId, gameId);
    onActivateGame?.(gameId);
  };

  return { sortBy, games, selectedGameId, handleSort, handleSelect, handleActivate };
}
