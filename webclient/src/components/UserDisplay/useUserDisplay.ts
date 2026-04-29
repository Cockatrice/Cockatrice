import { useState } from 'react';

import { useWebClient } from '@app/hooks';
import { ServerSelectors, useAppSelector } from '@app/store';

export interface UserDisplay {
  position: { x: number; y: number } | null;
  isABuddy: boolean;
  isIgnored: boolean;
  handleClick: (event: React.MouseEvent) => void;
  handleClose: () => void;
  onAddBuddy: () => void;
  onRemoveBuddy: () => void;
  onAddIgnore: () => void;
  onRemoveIgnore: () => void;
}

export function useUserDisplay(userName: string): UserDisplay {
  const buddyList = useAppSelector((state) => ServerSelectors.getBuddyList(state));
  const ignoreList = useAppSelector((state) => ServerSelectors.getIgnoreList(state));
  const [position, setPosition] = useState<{ x: number; y: number } | null>(null);
  const webClient = useWebClient();

  const handleClick = (event: React.MouseEvent) => {
    event.preventDefault();
    setPosition({ x: event.clientX + 2, y: event.clientY + 4 });
  };

  const handleClose = () => setPosition(null);

  const isABuddy = Boolean(buddyList[userName]);
  const isIgnored = Boolean(ignoreList[userName]);

  const onAddBuddy = () => {
    webClient.request.session.addToBuddyList(userName);
    handleClose();
  };
  const onRemoveBuddy = () => {
    webClient.request.session.removeFromBuddyList(userName);
    handleClose();
  };
  const onAddIgnore = () => {
    webClient.request.session.addToIgnoreList(userName);
    handleClose();
  };
  const onRemoveIgnore = () => {
    webClient.request.session.removeFromIgnoreList(userName);
    handleClose();
  };

  return {
    position,
    isABuddy,
    isIgnored,
    handleClick,
    handleClose,
    onAddBuddy,
    onRemoveBuddy,
    onAddIgnore,
    onRemoveIgnore,
  };
}
