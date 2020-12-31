// eslint-disable-next-line
import React, { Component } from "react";
import { connect } from "react-redux";
import { NavLink, generatePath } from "react-router-dom";

import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";

import { SessionService } from "AppShell/common/services";
import { RouteEnum } from "AppShell/common/types";

import { Selectors } from "store/server";

import { User } from "types";

import "./UserDisplay.css";


class UserDisplay extends Component<UserDisplayProps, UserDisplayState> {
  constructor(props) {
    super(props);

    this.handleClick = this.handleClick.bind(this);
    this.handleClose = this.handleClose.bind(this);
    this.navigateToUserProfile = this.navigateToUserProfile.bind(this);
    this.addToBuddyList = this.addToBuddyList.bind(this);
    this.removeFromBuddyList = this.removeFromBuddyList.bind(this);
    this.addToIgnoreList = this.addToIgnoreList.bind(this);
    this.removeFromIgnoreList = this.removeFromIgnoreList.bind(this);

    this.isABuddy = this.isABuddy.bind(this);
    this.isIgnored = this.isIgnored.bind(this);

    this.state = {
      position: null
    };
  }

  handleClick(event) {
    event.preventDefault();

    this.setState({
      position: {
        x: event.clientX + 2,
        y: event.clientY + 4,
      }
    });
  }

  handleClose() {
    this.setState({
      position: null
    });
  }

  navigateToUserProfile() {
    this.handleClose();
  }

  addToBuddyList() {
    SessionService.addToBuddyList(this.props.user.name);
    this.handleClose();
  }

  removeFromBuddyList() {
    SessionService.removeFromBuddyList(this.props.user.name);
    this.handleClose();
  }

  addToIgnoreList() {
    SessionService.addToIgnoreList(this.props.user.name);
    this.handleClose();
  }

  removeFromIgnoreList() {
    SessionService.removeFromIgnoreList(this.props.user.name);
    this.handleClose();
  }

  isABuddy() {
    return this.props.buddyList.filter(user => user.name === this.props.user.name).length;
  }

  isIgnored() {
    return this.props.ignoreList.filter(user => user.name === this.props.user.name).length;
  }

  render() {
    const { user } = this.props;
    const { position } = this.state;
    const { name } = user;

    const isABuddy = this.isABuddy();
    const isIgnored = this.isIgnored();


    console.log('user', name, !!isABuddy, !!isIgnored);

    return (
      <div className="user-display">
        <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="plain-link">
          <div className="user-display__details" onContextMenu={this.handleClick}>
            <div className="user-display__country"></div>
            <div className="user-display__name single-line-ellipsis">{name}</div>
          </div>
        </NavLink>
        <div className="user-display__menu">
          <Menu
            open={Boolean(position)}
            onClose={this.handleClose}
            anchorReference='anchorPosition'
            anchorPosition={
              position !== null
                ? { top: position.y, left: position.x }
                : undefined
            }
          >
            <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="user-display__link plain-link">
              <MenuItem dense>Chat</MenuItem>
            </NavLink>
            {
              !isABuddy
                ? ( <MenuItem dense onClick={this.addToBuddyList}>Add to Buddy List</MenuItem> )
                : ( <MenuItem dense onClick={this.removeFromBuddyList}>Remove From Buddy List</MenuItem> )
            }
            {
              !isIgnored
                ? ( <MenuItem dense onClick={this.addToIgnoreList}>Add to Ignore List</MenuItem> )
                : ( <MenuItem dense onClick={this.removeFromIgnoreList}>Remove From Ignore List</MenuItem> )
            }
          </Menu>
        </div>
      </div>
    );
  }
}

interface UserDisplayProps {
  user: User;
  buddyList: User[];
  ignoreList: User[];
}

interface UserDisplayState {
  position: any;
}

const mapStateToProps = (state) => ({
  buddyList: Selectors.getBuddyList(state),
  ignoreList: Selectors.getIgnoreList(state)
});

export default connect(mapStateToProps)(UserDisplay);
