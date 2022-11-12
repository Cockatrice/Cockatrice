// eslint-disable-next-line
import React, { Component } from "react";
import { useTranslation } from 'react-i18next';
import { connect } from 'react-redux';

import Button from '@mui/material/Button';
import ListItem from '@mui/material/ListItem';
import Paper from '@mui/material/Paper';

import { UserDisplay, VirtualList, AuthGuard, LanguageDropdown } from 'components';
import { AuthenticationService, SessionService } from 'api';
import { ServerSelectors } from 'store';
import { User } from 'types';
import Layout from 'containers/Layout/Layout';

import AddToBuddies from './AddToBuddies';
import AddToIgnore from './AddToIgnore';

import './Account.css';

const Account = (props: AccountProps) => {
  const { buddyList, ignoreList, serverName, serverVersion, user } = props;
  const { country, realName, name, userLevel, accountageSecs, avatarBmp } = user;
  let url = URL.createObjectURL(new Blob([avatarBmp], { 'type': 'image/png' }));

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
            itemKey={(index, data) => buddyList[index].name }
            items={ buddyList.map(user => (
              <ListItem button dense>
                <UserDisplay user={user} />
              </ListItem>
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
            itemKey={(index, data) => ignoreList[index].name }
            items={ ignoreList.map(user => (
              <ListItem button dense>
                <UserDisplay user={user} />
              </ListItem>
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
          <p>Account Age: {accountageSecs}</p>
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

interface AccountProps {
  buddyList: User[];
  ignoreList: User[];
  serverName: string;
  serverVersion: string;
  user: User;
}

const mapStateToProps = state => ({
  buddyList: ServerSelectors.getBuddyList(state),
  ignoreList: ServerSelectors.getIgnoreList(state),
  serverName: ServerSelectors.getName(state),
  serverVersion: ServerSelectors.getVersion(state),
  user: ServerSelectors.getUser(state),
});

export default connect(mapStateToProps)(Account);
