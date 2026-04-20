import { NavLink, generatePath } from 'react-router-dom';

import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import { Images } from '@app/images';
import { App, Data } from '@app/types';

import { useUserDisplay } from './useUserDisplay';

import './UserDisplay.css';

interface UserDisplayProps {
  user: Data.ServerInfo_User;
}

const UserDisplay = ({ user }: UserDisplayProps) => {
  const { name, country } = user;
  const {
    position,
    isABuddy,
    isIgnored,
    handleClick,
    handleClose,
    onAddBuddy,
    onRemoveBuddy,
    onAddIgnore,
    onRemoveIgnore,
  } = useUserDisplay(name);

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

export default UserDisplay;
