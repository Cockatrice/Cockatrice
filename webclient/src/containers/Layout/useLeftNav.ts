import { useEffect, useState } from 'react';
import { useNavigate, generatePath } from 'react-router-dom';

import { useWebClient } from '@app/hooks';
import { RoomsSelectors, ServerSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

interface LeftNavState {
  anchorEl: Element | null;
  showCardImportDialog: boolean;
  options: string[];
}

export interface LeftNav {
  joinedRooms: any[];
  isConnected: boolean;
  state: LeftNavState;
  handleMenuOpen: (event: React.MouseEvent) => void;
  handleMenuItemClick: (option: string) => void;
  handleMenuClose: () => void;
  leaveRoom: (event: React.MouseEvent, roomId: number) => void;
  openImportCardWizard: () => void;
  closeImportCardWizard: () => void;
}

export function useLeftNav(): LeftNav {
  const joinedRooms = useAppSelector((state) => RoomsSelectors.getJoinedRooms(state));
  const isConnected = useAppSelector(ServerSelectors.getIsConnected);
  const isModerator = useAppSelector(ServerSelectors.getIsUserModerator);
  const navigate = useNavigate();
  const webClient = useWebClient();
  const [state, setState] = useState<LeftNavState>({
    anchorEl: null,
    showCardImportDialog: false,
    options: [],
  });

  useEffect(() => {
    let options: string[] = [
      'Account',
      'Replays',
    ];

    if (isModerator) {
      options = [
        ...options,
        'Administration',
        'Logs',
      ];
    }

    setState((s) => ({ ...s, options }));
  }, [isModerator]);

  const handleMenuOpen = (event: React.MouseEvent) => {
    setState((s) => ({ ...s, anchorEl: event.target as Element }));
  };

  const handleMenuItemClick = (option: string) => {
    const route = App.RouteEnum[option.toUpperCase()];
    navigate(generatePath(route));
  };

  const handleMenuClose = () => {
    setState((s) => ({ ...s, anchorEl: null }));
  };

  const leaveRoom = (event: React.MouseEvent, roomId: number) => {
    event.preventDefault();
    webClient.request.rooms.leaveRoom(roomId);
  };

  const openImportCardWizard = () => {
    setState((s) => ({ ...s, showCardImportDialog: true }));
    handleMenuClose();
  };

  const closeImportCardWizard = () => {
    setState((s) => ({ ...s, showCardImportDialog: false }));
  };

  return {
    joinedRooms,
    isConnected,
    state,
    handleMenuOpen,
    handleMenuItemClick,
    handleMenuClose,
    leaveRoom,
    openImportCardWizard,
    closeImportCardWizard,
  };
}
