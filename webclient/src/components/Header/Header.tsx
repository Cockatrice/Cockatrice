import React, { useState, useEffect } from 'react';
import { connect } from 'react-redux';
import { NavLink, useNavigate, generatePath } from 'react-router-dom';
import AppBar from '@material-ui/core/AppBar';
import IconButton from '@material-ui/core/IconButton';
import Menu from '@material-ui/core/Menu';
import MenuItem from '@material-ui/core/MenuItem';
import Toolbar from '@material-ui/core/Toolbar';
import ArrowDropDownIcon from '@material-ui/icons/ArrowDropDown';
import CloseIcon from '@material-ui/icons/Close';
import MailOutlineRoundedIcon from '@material-ui/icons/MailOutline';
import MenuRoundedIcon from '@material-ui/icons/MenuRounded';
import * as _ from 'lodash';

import { AuthenticationService, RoomsService } from 'api';
import { CardImportDialog } from 'dialogs';
import { Images } from 'images';
import { RoomsSelectors, ServerSelectors } from 'store';
import { Room, RouteEnum, User } from 'types';

import './Header.css';

const Header = ({ joinedRooms, serverState, user }: HeaderProps) => {
  const navigate = useNavigate();
  const [state, setState] = useState<HeaderState>({
    anchorEl: null,
    showCardImportDialog: false,
    options: [],
  });

  useEffect(() => {
    let options: string[] = [
      'Account',
      'Replays',
    ];

    if (user && AuthenticationService.isModerator(user)) {
      options = [
        ...options,
        'Administration',
        'Logs'
      ];
    }

    setState(s => ({ ...s, options }));
  }, [user]);

  const handleMenuOpen = (event) => {
    setState(s => ({ ...s, anchorEl: event.target }));
  }

  const handleMenuItemClick = (option: string) => {
    const route = RouteEnum[option.toUpperCase()];
    navigate(generatePath(route));
  }

  const handleMenuClose = () => {
    setState(s => ({ ...s, anchorEl: null }));
  }

  const leaveRoom = (event, roomId) => {
    event.preventDefault();
    RoomsService.leaveRoom(roomId);
  };

  const openImportCardWizard = () => {
    setState(s => ({ ...s, showCardImportDialog: true }));
    handleMenuClose();
  }

  const closeImportCardWizard = () => {
    setState(s => ({ ...s, showCardImportDialog: false }));
  }

  return (
    <AppBar className="Header" position="static">
      <Toolbar variant="dense">
        <div className="Header__logo">
          <NavLink to={RouteEnum.SERVER}>
            <img src={Images.Logo} alt="logo" />
          </NavLink>
          { AuthenticationService.isConnected(serverState) && (
            <span className="Header-server__indicator"></span>
          ) }
        </div>
        { AuthenticationService.isConnected(serverState) && (
          <div className="Header-content">
            <nav className="Header-nav">
              <nav className="Header-nav__links">
                <div className="Header-nav__link">
                  <NavLink
                    className="Header-nav__link-btn"
                    to={
                      joinedRooms.length
                        ? generatePath(RouteEnum.ROOM, { roomId: joinedRooms[0].roomId.toString() })
                        : RouteEnum.SERVER
                    }
                  >
                    Rooms
                    <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                  <div className="Header-nav__link-menu">
                    {joinedRooms.map(({ name, roomId }) => (
                      <MenuItem className="Header-nav__link-menu__item" key={roomId}>
                        <NavLink className="Header-nav__link-menu__btn" to={ generatePath(RouteEnum.ROOM, { roomId: roomId.toString() }) }>
                          {name}

                          <IconButton size="small" edge="end" onClick={event => leaveRoom(event, roomId)}>
                            <CloseIcon style={{ fontSize: 10, color: 'white' }} />
                          </IconButton>
                        </NavLink>
                      </MenuItem>
                    ))}
                  </div>
                </div>
                <div className="Header-nav__link">
                  <NavLink className="Header-nav__link-btn" to={ RouteEnum.GAME }>
                    Games
                    <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
                <div className="Header-nav__link">
                  <NavLink className="Header-nav__link-btn" to={ RouteEnum.DECKS }>
                    Decks
                    <ArrowDropDownIcon className="Header-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
              </nav>
              <div className="Header-nav__actions">
                <div className="Header-nav__action">
                  <IconButton>
                    <MailOutlineRoundedIcon style={{ color: 'inherit' }} />
                  </IconButton>
                </div>
                <div className="Header-nav__action">
                  <IconButton onClick={handleMenuOpen}>
                    <MenuRoundedIcon style={{ color: 'inherit' }} />
                  </IconButton>
                  <Menu
                    anchorEl={state.anchorEl}
                    keepMounted
                    open={!!state.anchorEl}
                    onClose={() => handleMenuClose()}
                    PaperProps={{
                      style: {
                        marginTop: '32px',
                        width: '20ch',
                      },
                    }}
                  >
                    {state.options.map((option) => (
                      <MenuItem key={option} onClick={(event) => handleMenuItemClick(option)}>
                        {option}
                      </MenuItem>
                    ))}

                    <MenuItem key='Import Cards' onClick={(event) => openImportCardWizard()}>
                      Import Cards
                    </MenuItem>
                  </Menu>
                </div>
              </div>
            </nav>
          </div>
        ) }
      </Toolbar>

      <CardImportDialog
        isOpen={state.showCardImportDialog}
        handleClose={closeImportCardWizard}
      ></CardImportDialog>
    </AppBar>
  )
}

interface HeaderProps {
  serverState: number;
  server: string;
  user: User;
  joinedRooms: Room[];
}

interface HeaderState {
  anchorEl: Element;
  showCardImportDialog: boolean;
  options: string[];
}

const mapStateToProps = state => ({
  serverState: ServerSelectors.getState(state),
  server: ServerSelectors.getName(state),
  user: ServerSelectors.getUser(state),
  joinedRooms: RoomsSelectors.getJoinedRooms(state),
});

export default connect(mapStateToProps)(Header);
