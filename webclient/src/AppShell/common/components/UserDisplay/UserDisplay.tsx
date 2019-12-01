// eslint-disable-next-line
import React from "react";
import { NavLink, generatePath } from "react-router-dom";

import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";

import { RouteEnum } from "AppShell/common/types";

import "./UserDisplay.css";


const UserDisplay = ({ user }) => {
  const [state, setState] = React.useState(null);
  const { name } = user;

  function handleClick(event) {
    event.preventDefault();

    setState({
      x: event.clientX + 2,
      y: event.clientY + 4,
    });
  }

  function handleClose() {
    setState(null);
  }

  function navigateToUserProfile() {
    handleClose();
  }

  function addToBuddyList() {
    handleClose();
  }

  function addToIgnoreList() {
    handleClose();
  }

  return (
    <div className="user-display">
      <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="plain-link">
        <div className="user-display__details" onContextMenu={handleClick}>
          <div className="user-display__country"></div>
          <div className="user-display__name single-line-ellipsis">{name}</div>
        </div>
      </NavLink>
      <div className="user-display__menu">
        <Menu
          open={Boolean(state)}
          onClose={handleClose}
          anchorReference='anchorPosition'
          anchorPosition={
            state !== null
              ? { top: state.y, left: state.x }
              : undefined
          }
        >
          <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="user-display__link plain-link">
            <MenuItem dense>Chat</MenuItem>
          </NavLink>
          <MenuItem dense onClick={addToBuddyList}>Add to Buddy List</MenuItem>
          <MenuItem dense onClick={addToIgnoreList}>Add to Ignore List</MenuItem>
        </Menu>
      </div>
    </div>
  );
}

export default UserDisplay;
