import { useEffect, useMemo } from 'react';
import { useParams } from 'react-router-dom';

import { useWebClient } from '@app/hooks';
import { ServerSelectors, useAppSelector } from '@app/store';
import type { Data } from '@app/types';

export interface PlayerViewModel {
  /** Resolved username from the route; null when the `:name` param is missing. */
  name: string | null;
  /** Profile record from server.userInfo[name]; undefined until the server response lands. */
  userInfo: Data.ServerInfo_User | undefined;
  /** The logged-in user, for self-profile and mod-permission checks. */
  currentUser: Data.ServerInfo_User | null;
  isSelf: boolean;
  isABuddy: boolean;
  isIgnored: boolean;
  isModerator: boolean;

  onAddBuddy: () => void;
  onRemoveBuddy: () => void;
  onAddIgnore: () => void;
  onRemoveIgnore: () => void;
  onSendMessage: (message: string) => void;
  onWarnUser: (reason: string) => void;
  onBanFromServer: (minutes: number, reason: string, visibleReason?: string) => void;
}

/**
 * Drives the Player container: resolves the `:name` route param, dispatches
 * `getUserInfo` on mount so the server populates `server.userInfo[name]`, and
 * exposes the buddy/ignore/mod-action callbacks desktop surfaces in UserInfoBox.
 */
export function usePlayer(): PlayerViewModel {
  const webClient = useWebClient();
  const params = useParams<{ name?: string }>();
  const name = params.name ?? null;

  const userInfo = useAppSelector((state) =>
    name ? ServerSelectors.getUserInfoByName(state, name) : undefined,
  );
  const currentUser = useAppSelector(ServerSelectors.getUser);
  const buddyList = useAppSelector(ServerSelectors.getBuddyList);
  const ignoreList = useAppSelector(ServerSelectors.getIgnoreList);
  const isModerator = useAppSelector(ServerSelectors.getIsUserModerator);

  useEffect(() => {
    if (name) {
      webClient.request.session.getUserInfo(name);
    }
  }, [name, webClient]);

  const { isSelf, isABuddy, isIgnored } = useMemo(() => ({
    isSelf: Boolean(currentUser && name && currentUser.name === name),
    isABuddy: Boolean(name && buddyList[name]),
    isIgnored: Boolean(name && ignoreList[name]),
  }), [currentUser, name, buddyList, ignoreList]);

  const onAddBuddy = () => name && webClient.request.session.addToBuddyList(name);
  const onRemoveBuddy = () => name && webClient.request.session.removeFromBuddyList(name);
  const onAddIgnore = () => name && webClient.request.session.addToIgnoreList(name);
  const onRemoveIgnore = () => name && webClient.request.session.removeFromIgnoreList(name);
  const onSendMessage = (message: string) => name && webClient.request.session.sendDirectMessage(name, message);
  const onWarnUser = (reason: string) => name && webClient.request.moderator.warnUser(name, reason);
  const onBanFromServer = (minutes: number, reason: string, visibleReason?: string) =>
    name && webClient.request.moderator.banFromServer(minutes, name, undefined, reason, visibleReason);

  return {
    name,
    userInfo,
    currentUser,
    isSelf,
    isABuddy,
    isIgnored,
    isModerator,
    onAddBuddy,
    onRemoveBuddy,
    onAddIgnore,
    onRemoveIgnore,
    onSendMessage,
    onWarnUser,
    onBanFromServer,
  };
}
