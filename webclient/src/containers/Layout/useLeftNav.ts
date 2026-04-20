import { useMemo, useState } from 'react';
import { useNavigate, generatePath } from 'react-router-dom';

import { useWebClient } from '@app/hooks';
import { RoomsSelectors, ServerSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

export interface LeftNavOption {
  label: string;
  route: App.RouteEnum;
}

interface LeftNavState {
  anchorEl: Element | null;
  showCardImportDialog: boolean;
  options: LeftNavOption[];
}

export interface LeftNav {
  joinedRooms: ReturnType<typeof RoomsSelectors.getJoinedRooms>;
  isConnected: boolean;
  state: LeftNavState;
  handleMenuOpen: (event: React.MouseEvent) => void;
  handleMenuItemClick: (option: LeftNavOption) => void;
  handleMenuClose: () => void;
  leaveRoom: (event: React.MouseEvent, roomId: number) => void;
  openImportCardWizard: () => void;
  closeImportCardWizard: () => void;
}

const BASE_OPTIONS: LeftNavOption[] = [
  { label: 'Account', route: App.RouteEnum.ACCOUNT },
  { label: 'Replays', route: App.RouteEnum.REPLAYS },
];

const MODERATOR_OPTIONS: LeftNavOption[] = [
  { label: 'Administration', route: App.RouteEnum.ADMINISTRATION },
  { label: 'Logs', route: App.RouteEnum.LOGS },
];

export function useLeftNav(): LeftNav {
  const joinedRooms = useAppSelector((state) => RoomsSelectors.getJoinedRooms(state));
  const isConnected = useAppSelector(ServerSelectors.getIsConnected);
  const isModerator = useAppSelector(ServerSelectors.getIsUserModerator);
  const navigate = useNavigate();
  const webClient = useWebClient();
  const [anchorEl, setAnchorEl] = useState<Element | null>(null);
  const [showCardImportDialog, setShowCardImportDialog] = useState(false);

  const options = useMemo<LeftNavOption[]>(
    () => (isModerator ? [...BASE_OPTIONS, ...MODERATOR_OPTIONS] : BASE_OPTIONS),
    [isModerator],
  );

  const state: LeftNavState = { anchorEl, showCardImportDialog, options };

  const handleMenuOpen = (event: React.MouseEvent) => {
    setAnchorEl(event.target as Element);
  };

  const handleMenuItemClick = (option: LeftNavOption) => {
    navigate(generatePath(option.route));
  };

  const handleMenuClose = () => {
    setAnchorEl(null);
  };

  const leaveRoom = (event: React.MouseEvent, roomId: number) => {
    event.preventDefault();
    webClient.request.rooms.leaveRoom(roomId);
  };

  const openImportCardWizard = () => {
    setShowCardImportDialog(true);
    handleMenuClose();
  };

  const closeImportCardWizard = () => {
    setShowCardImportDialog(false);
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
