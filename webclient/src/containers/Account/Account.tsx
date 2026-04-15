// eslint-disable-next-line
import React, { Component } from "react";
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import ListItemButton from '@mui/material/ListItemButton';
import Paper from '@mui/material/Paper';

import { UserDisplay, VirtualList, AuthGuard, LanguageDropdown } from '@app/components';
import { AuthenticationService, SessionService } from '@app/api';
import { ServerSelectors } from '@app/store';
import Layout from '../Layout/Layout';
import { useAppSelector } from '@app/store';

import AddToBuddies from './AddToBuddies';
import AddToIgnore from './AddToIgnore';

import './Account.css';

const Account = () => {
  const buddyList = useAppSelector(state => ServerSelectors.getBuddyList(state));
  const ignoreList = useAppSelector(state => ServerSelectors.getIgnoreList(state));
  const serverName = useAppSelector(state => ServerSelectors.getName(state));
  const serverVersion = useAppSelector(state => ServerSelectors.getVersion(state));
  const user = useAppSelector(state => ServerSelectors.getUser(state));
  const { country, realName, name, userLevel, accountageSecs, avatarBmp } = user || {};
  let url = URL.createObjectURL(new Blob([avatarBmp as BlobPart], { 'type': 'image/png' }));

  const { t } = useTranslation();

  const handleAddToBuddies = ({ userName }) => {
    SessionService.addToBuddyList(userName);
  };

  const handleAddToIgnore = ({ userName }) => {
    SessionService.addToIgnoreList(userName);
  };

  return (
    <Layout className="account">
      <AuthGuard />
      <div className="account-column">
        <Paper className="account-list">
          <div className="">
            Buddies Online: ?/{buddyList.length}
          </div>
          <VirtualList
            items={ buddyList.map(user => (
              <ListItemButton dense>
                <UserDisplay user={user} />
              </ListItemButton>
            )) }
          />
          <div className="" style={{ borderTop: '1px solid' }}>
            <AddToBuddies onSubmit={handleAddToBuddies} />
          </div>
        </Paper>
      </div>
      <div className="account-column">
        <Paper className="account-list overflow-scroll">
          <div className="">
            Ignored Users Online: ?/{ignoreList.length}
          </div>
          <VirtualList
            items={ ignoreList.map(user => (
              <ListItemButton dense>
                <UserDisplay user={user} />
              </ListItemButton>
            )) }
          />
          <div className="" style={{ borderTop: '1px solid' }}>
            <AddToIgnore onSubmit={handleAddToIgnore} />
          </div>
        </Paper>
      </div>
      <div className="account-column overflow-scroll">
        <Paper className="account-details" style={{ margin: '0 0 5px 0' }}>
          <img src={url} alt={name} />
          <p><strong>{name}</strong></p>
          <p>Location: ({country?.toUpperCase()})</p>
          <p>User Level: {userLevel}</p>
          <p>Account Age: {String(accountageSecs)}</p>
          <p>Real Name: {realName}</p>
          <div className="account-details__actions">
            <Button size="small" color="primary" variant="contained">Edit</Button>
            <Button size="small" color="primary" variant="contained">Change<br />Password</Button>
            <Button size="small" color="primary" variant="contained">Change<br />Avatar</Button>
          </div>

        </Paper>
        <Paper className="account-details">
          <p>Server Name: {serverName}</p>
          <p>Server Version: {serverVersion}</p>
          <Button color="primary" variant="contained" onClick={() => AuthenticationService.disconnect()}>{ t('Common.disconnect') }</Button>

          <div className="account-details__lang">
            <LanguageDropdown />
          </div>
        </Paper>
      </div>
    </Layout>
  )
}

export default Account;
