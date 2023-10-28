import React, { useState, useEffect } from 'react';
import { connect } from 'react-redux';
import { NavLink, useNavigate, generatePath } from 'react-router-dom';
import IconButton from '@mui/material/IconButton';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import ArrowDropDownIcon from '@mui/icons-material/ArrowDropDown';
import CloseIcon from '@mui/icons-material/Close';
import MailOutlineRoundedIcon from '@mui/icons-material/MailOutline';
import MenuRoundedIcon from '@mui/icons-material/MenuRounded';
import * as _ from 'lodash';

import { AuthenticationService, RoomsService } from 'api';
import { CardImportDialog } from 'dialogs';
import { Images } from 'images';
import { RoomsSelectors, ServerSelectors } from 'store';
import { Room, RouteEnum, User } from 'types';

import './LeftNav.css';

const LeftNav = ({ joinedRooms, serverState, user }: LeftNavProps) => {
  const navigate = useNavigate();
  const [state, setState] = useState<LeftNavState>({
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
    <div className="LeftNav__container">
      <div>
        <div className="LeftNav__logo">
          <NavLink to={RouteEnum.SERVER}>
            <img src={Images.Logo} alt="logo" />
          </NavLink>
          { AuthenticationService.isConnected(serverState) && (
            <span className="LeftNav-server__indicator"></span>
          ) }
        </div>
        { AuthenticationService.isConnected(serverState) && (
          <div className="LeftNav-content">
            <nav className="LeftNav-nav">
              <nav className="LeftNav-nav__links">
                <div className="LeftNav-nav__link">
                  <NavLink
                    className="LeftNav-nav__link-btn"
                    to={
                      joinedRooms.length
                        ? generatePath(RouteEnum.ROOM, { roomId: joinedRooms[0].roomId.toString() })
                        : RouteEnum.SERVER
                    }
                  >
                    Rooms
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                  <div className="LeftNav-nav__link-menu">
                    {joinedRooms.map(({ name, roomId }) => (
                      <div className="LeftNav-nav__link-menu__item" key={roomId}>
                        <NavLink className="LeftNav-nav__link-menu__btn" to={ generatePath(RouteEnum.ROOM, { roomId: roomId.toString() }) }>
                          {name}

                          <IconButton size="small" edge="end" onClick={event => leaveRoom(event, roomId)}>
                            <CloseIcon style={{ fontSize: 10, color: 'white' }} />
                          </IconButton>
                        </NavLink>
                      </div>
                    ))}
                  </div>
                </div>
                <div className="LeftNav-nav__link">
                  <NavLink className="LeftNav-nav__link-btn" to={ RouteEnum.GAME }>
                    Games
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
                <div className="LeftNav-nav__link">
                  <NavLink className="LeftNav-nav__link-btn" to={ RouteEnum.DECKS }>
                    Decks
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
              </nav>
              <div className="LeftNav-nav__actions">
                <div className="LeftNav-nav__action">
                  <IconButton size="large">
                    <MailOutlineRoundedIcon style={{ color: 'inherit' }} />
                  </IconButton>
                </div>
                <div className="LeftNav-nav__action">
                  <IconButton onClick={handleMenuOpen} size="large">
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
      </div>

      <CardImportDialog
        isOpen={state.showCardImportDialog}
        handleClose={closeImportCardWizard}
      ></CardImportDialog>
    </div>
  );
}

interface LeftNavProps {
  serverState: number;
  server: string;
  user: User;
  joinedRooms: Room[];
  showNav?: boolean;
}

interface LeftNavState {
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

export default connect(mapStateToProps)(LeftNav);
