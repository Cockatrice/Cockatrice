
import React, { useState } from 'react';
import { NavLink, generatePath } from 'react-router-dom';

import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import { Images } from '@app/images';
import { request } from '@app/api';
import { ServerSelectors } from '@app/store';
import { App, Data } from '@app/types';
import { useAppSelector } from '@app/store';

import './UserDisplay.css';


const UserDisplay = ({ user }: UserDisplayProps) => {
  const buddyList = useAppSelector(state => ServerSelectors.getBuddyList(state));
  const ignoreList = useAppSelector(state => ServerSelectors.getIgnoreList(state));
  const [position, setPosition] = useState<{ x: number; y: number } | null>(null);

  const { name, country } = user;

  const handleClick = (event) => {
    event.preventDefault();
    setPosition({ x: event.clientX + 2, y: event.clientY + 4 });
  };

  const handleClose = () => setPosition(null);

  const isABuddy = Boolean(buddyList[user.name]);
  const isIgnored = Boolean(ignoreList[user.name]);

  const onAddBuddy = () => {
    request.session.addToBuddyList(user.name);
    handleClose();
  };
  const onRemoveBuddy = () => {
    request.session.removeFromBuddyList(user.name);
    handleClose();
  };
  const onAddIgnore = () => {
    request.session.addToIgnoreList(user.name);
    handleClose();
  };
  const onRemoveIgnore = () => {
    request.session.removeFromIgnoreList(user.name);
    handleClose();
  };

  return (
    <div className="user-display">
      <NavLink to={generatePath(App.RouteEnum.PLAYER, { name })} className="plain-link">
        <div className="user-display__details" onContextMenu={handleClick}>
          <img className="user-display__country" src={Images.Countries[country]} alt={country}></img>
          <div className="user-display__name single-line-ellipsis">{name}</div>
        </div>
      </NavLink>
      <div className="user-display__menu">
        <Menu
          open={Boolean(position)}
          onClose={handleClose}
          anchorReference='anchorPosition'
          anchorPosition={
            position !== null
              ? { top: position.y, left: position.x }
              : undefined
          }
        >
          <NavLink to={generatePath(App.RouteEnum.PLAYER, { name })} className="user-display__link plain-link">
            <MenuItem dense>Chat</MenuItem>
          </NavLink>
          {
            !isABuddy
              ? (<MenuItem dense onClick={onAddBuddy}>Add to Buddy List</MenuItem>)
              : (<MenuItem dense onClick={onRemoveBuddy}>Remove From Buddy List</MenuItem>)
          }
          {
            !isIgnored
              ? (<MenuItem dense onClick={onAddIgnore}>Add to Ignore List</MenuItem>)
              : (<MenuItem dense onClick={onRemoveIgnore}>Remove From Ignore List</MenuItem>)
          }
        </Menu>
      </div>
    </div>
  );
};

interface UserDisplayProps {
  user: Data.ServerInfo_User;
}

export default UserDisplay;
