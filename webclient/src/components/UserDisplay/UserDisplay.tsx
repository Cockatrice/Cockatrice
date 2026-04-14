
import React, { useState } from 'react';
import { NavLink, generatePath } from 'react-router-dom';

import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import { Images } from 'images/Images';
import { SessionService } from 'api';
import { ServerSelectors } from 'store';
import { RouteEnum, User } from 'types';
import { useAppSelector } from 'store/store';

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

  const isABuddy = buddyList.filter(u => u.name === user.name).length;
  const isIgnored = ignoreList.filter(u => u.name === user.name).length;

  const onAddBuddy = () => {
    SessionService.addToBuddyList(user.name);
    handleClose();
  };
  const onRemoveBuddy = () => {
    SessionService.removeFromBuddyList(user.name);
    handleClose();
  };
  const onAddIgnore = () => {
    SessionService.addToIgnoreList(user.name);
    handleClose();
  };
  const onRemoveIgnore = () => {
    SessionService.removeFromIgnoreList(user.name);
    handleClose();
  };

  return (
    <div className="user-display">
      <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="plain-link">
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
          <NavLink to={generatePath(RouteEnum.PLAYER, { name })} className="user-display__link plain-link">
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
  user: User;
}

export default UserDisplay;
