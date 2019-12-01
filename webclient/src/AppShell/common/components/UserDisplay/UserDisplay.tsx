// eslint-disable-next-line
import React from "react";

import Menu from "@material-ui/core/Menu";
import MenuItem from "@material-ui/core/MenuItem";

import "./UserDisplay.css";


const UserDisplay = ({ user }) => {
  const [state, setState] = React.useState(null);

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
      <div className="user-display__details" onContextMenu={handleClick}>
        <div className="user-display__country"></div>
        <div className="user-display__name single-line-ellipsis">{user.name}</div>
      </div>
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
          <MenuItem onClick={navigateToUserProfile}>Profile</MenuItem>
          <MenuItem onClick={addToBuddyList}>Add to Buddy List</MenuItem>
          <MenuItem onClick={addToIgnoreList}>Add to Ignore List</MenuItem>
        </Menu>
      </div>
    </div>
  );
}

export default UserDisplay;
