import { useEffect, useMemo } from 'react';

import { useWebClient } from '@app/hooks';
import { ServerSelectors, useAppSelector } from '@app/store';

export interface Account {
  buddyList: any[];
  ignoreList: any[];
  serverName: string | undefined;
  serverVersion: string | undefined;
  user: any;
  avatarUrl: string;
  handleAddToBuddies: (args: { userName: string }) => void;
  handleAddToIgnore: (args: { userName: string }) => void;
  handleDisconnect: () => void;
}

export function useAccount(): Account {
  const buddyList = useAppSelector((state) => ServerSelectors.getSortedBuddyList(state));
  const ignoreList = useAppSelector((state) => ServerSelectors.getSortedIgnoreList(state));
  const serverName = useAppSelector((state) => ServerSelectors.getName(state));
  const serverVersion = useAppSelector((state) => ServerSelectors.getVersion(state));
  const user = useAppSelector((state) => ServerSelectors.getUser(state));
  const webClient = useWebClient();
  const { avatarBmp } = user || {};

  const avatarUrl = useMemo(() => {
    if (!avatarBmp) {
      return '';
    }
    return URL.createObjectURL(new Blob([avatarBmp as BlobPart], { type: 'image/png' }));
  }, [avatarBmp]);

  useEffect(() => {
    return () => {
      if (avatarUrl) {
        URL.revokeObjectURL(avatarUrl);
      }
    };
  }, [avatarUrl]);

  const handleAddToBuddies = ({ userName }: { userName: string }) => {
    webClient.request.session.addToBuddyList(userName);
  };

  const handleAddToIgnore = ({ userName }: { userName: string }) => {
    webClient.request.session.addToIgnoreList(userName);
  };

  const handleDisconnect = () => {
    webClient.request.authentication.disconnect();
  };

  return {
    buddyList,
    ignoreList,
    serverName,
    serverVersion,
    user,
    avatarUrl,
    handleAddToBuddies,
    handleAddToIgnore,
    handleDisconnect,
  };
}
