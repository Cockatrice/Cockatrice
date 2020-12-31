// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";

import Button from "@material-ui/core/Button";
import ListItem from "@material-ui/core/ListItem";
import Paper from "@material-ui/core/Paper";

import UserDisplay from "AppShell/common/components/UserDisplay/UserDisplay";
import VirtualList from "AppShell/common/components/VirtualList/VirtualList";

import { AuthenticationService, SessionService } from "AppShell/common/services";

import AuthGuard from "AppShell/common/guards/AuthGuard";

import { Selectors } from "store/server";

import { User } from 'types';

import AddToBuddies from './AddToBuddies/AddToBuddies';
import AddToIgnore from './AddToIgnore/AddToIgnore';

import "./Account.css";

class Account extends Component<AccountProps> {
  handleAddToBuddies({ userName }) {
    SessionService.addToBuddyList(userName);
  }

  handleAddToIgnore({ userName }) {
    SessionService.addToIgnoreList(userName);
  }

  render() {
    console.log(this.props);

    const { buddyList, ignoreList, serverName, serverVersion, user } = this.props;
    const { country, realName, name, userLevel, accountageSecs, avatarBmp } = user;

    var url = URL.createObjectURL(new Blob([avatarBmp], {'type': 'image/png'}));

    return (
      <div className="account">
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
              ) ) }
            />
            <div className="" style={{borderTop: "1px solid"}}>
              <AddToBuddies onSubmit={this.handleAddToBuddies} />
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
              ) ) }
            />
            <div className="" style={{borderTop: "1px solid"}}>
              <AddToIgnore onSubmit={this.handleAddToIgnore} />
            </div>
          </Paper>
        </div>
        <div className="account-column overflow-scroll">
          <Paper className="account-details" style={{margin: "0 0 5px 0"}}>
            <img src={url} alt={name} />
            <p><strong>{name}</strong></p>
            <p>Location: ({country.toUpperCase()})</p>
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
            <Button color="primary" variant="contained" onClick={() => AuthenticationService.disconnect()}>Disconnect</Button>
          </Paper>
        </div>
      </div>  
    )
  }
}

interface AccountProps {
  buddyList: User[];
  ignoreList: User[];
  serverName: string;
  serverVersion: string;
  user: User;
}

const mapStateToProps = state => ({
  buddyList: Selectors.getBuddyList(state),
  ignoreList: Selectors.getIgnoreList(state),
  serverName: Selectors.getName(state),
  serverVersion: Selectors.getVersion(state),
  user: Selectors.getUser(state),
});

export default connect(mapStateToProps)(Account);
