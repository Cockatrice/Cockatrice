import React, { useState, useEffect } from 'react';
import { NavLink, useNavigate, generatePath } from 'react-router-dom';
import IconButton from '@mui/material/IconButton';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import ArrowDropDownIcon from '@mui/icons-material/ArrowDropDown';
import CloseIcon from '@mui/icons-material/Close';
import MailOutlineRoundedIcon from '@mui/icons-material/MailOutlineRounded';
import MenuRoundedIcon from '@mui/icons-material/MenuRounded';

import { CardImportDialog } from '@app/dialogs';
import { useWebClient } from '@app/hooks';
import { Images } from '@app/images';
import { RoomsSelectors, ServerSelectors } from '@app/store';
import { App } from '@app/types';
import { useAppSelector } from '@app/store';

import './LeftNav.css';

interface LeftNavState {
  anchorEl: Element;
  showCardImportDialog: boolean;
  options: string[];
}

const LeftNav = () => {
  const joinedRooms = useAppSelector(state => RoomsSelectors.getJoinedRooms(state));
  const isConnected = useAppSelector(ServerSelectors.getIsConnected);
  const isModerator = useAppSelector(ServerSelectors.getIsUserModerator);
  const navigate = useNavigate();
  const webClient = useWebClient();
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

    if (isModerator) {
      options = [
        ...options,
        'Administration',
        'Logs'
      ];
    }

    setState(s => ({ ...s, options }));
  }, [isModerator]);

  const handleMenuOpen = (event) => {
    setState(s => ({ ...s, anchorEl: event.target }));
  }

  const handleMenuItemClick = (option: string) => {
    const route = App.RouteEnum[option.toUpperCase()];
    navigate(generatePath(route));
  }

  const handleMenuClose = () => {
    setState(s => ({ ...s, anchorEl: null }));
  }

  const leaveRoom = (event, roomId) => {
    event.preventDefault();
    webClient.request.rooms.leaveRoom(roomId);
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
          <NavLink to={App.RouteEnum.SERVER}>
            <img src={Images.Logo} alt="logo" />
          </NavLink>
          { isConnected && (
            <span className="LeftNav-server__indicator"></span>
          ) }
        </div>
        { isConnected && (
          <div className="LeftNav-content">
            <nav className="LeftNav-nav">
              <nav className="LeftNav-nav__links">
                <div className="LeftNav-nav__link">
                  <NavLink
                    className="LeftNav-nav__link-btn"
                    to={
                      joinedRooms.length
                        ? generatePath(App.RouteEnum.ROOM, { roomId: joinedRooms[0].info.roomId.toString() })
                        : App.RouteEnum.SERVER
                    }
                  >
                    Rooms
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                  <div className="LeftNav-nav__link-menu">
                    {joinedRooms.map((room) => (
                      <div className="LeftNav-nav__link-menu__item" key={room.info.roomId}>
                        <NavLink className="LeftNav-nav__link-menu__btn"
                          to={ generatePath(App.RouteEnum.ROOM, { roomId: room.info.roomId.toString() }) }
                        >
                          {room.info.name}

                          <IconButton size="small" edge="end" onClick={event => leaveRoom(event, room.info.roomId)}>
                            <CloseIcon style={{ fontSize: 10, color: 'white' }} />
                          </IconButton>
                        </NavLink>
                      </div>
                    ))}
                  </div>
                </div>
                <div className="LeftNav-nav__link">
                  <NavLink className="LeftNav-nav__link-btn" to={ App.RouteEnum.GAME }>
                    Games
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
                <div className="LeftNav-nav__link">
                  <NavLink className="LeftNav-nav__link-btn" to={ App.RouteEnum.DECKS }>
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
                    slotProps={{
                      paper: {
                        style: {
                          marginTop: '32px',
                          width: '20ch',
                        },
                      },
                    }}
                  >
                    {state.options.map((option) => (
                      <MenuItem key={option} onClick={() => handleMenuItemClick(option)}>
                        {option}
                      </MenuItem>
                    ))}

                    <MenuItem key='Import Cards' onClick={() => openImportCardWizard()}>
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

export default LeftNav;
