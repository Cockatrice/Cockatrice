import { NavLink, generatePath } from 'react-router-dom';
import IconButton from '@mui/material/IconButton';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import ArrowDropDownIcon from '@mui/icons-material/ArrowDropDown';
import CloseIcon from '@mui/icons-material/Close';
import MailOutlineRoundedIcon from '@mui/icons-material/MailOutlineRounded';
import MenuRoundedIcon from '@mui/icons-material/MenuRounded';

import { CardImportDialog } from '@app/dialogs';
import { Images } from '@app/images';
import { App } from '@app/types';

import { useLeftNav } from './useLeftNav';

import './LeftNav.css';

const LeftNav = () => {
  const {
    joinedRooms,
    isConnected,
    state,
    handleMenuOpen,
    handleMenuItemClick,
    handleMenuClose,
    leaveRoom,
    openImportCardWizard,
    closeImportCardWizard,
  } = useLeftNav();

  return (
    <div className="LeftNav__container">
      <div>
        <div className="LeftNav__logo">
          <NavLink to={App.RouteEnum.SERVER}>
            <img src={Images.Logo} alt="logo" />
          </NavLink>
          {isConnected && (
            <span className="LeftNav-server__indicator"></span>
          )}
        </div>
        {isConnected && (
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
                          to={generatePath(App.RouteEnum.ROOM, { roomId: room.info.roomId.toString() })}
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
                  <NavLink className="LeftNav-nav__link-btn" to={App.RouteEnum.GAME}>
                    Games
                    <ArrowDropDownIcon className="LeftNav-nav__link-btn__icon" fontSize="small" />
                  </NavLink>
                </div>
                <div className="LeftNav-nav__link">
                  <NavLink className="LeftNav-nav__link-btn" to={App.RouteEnum.DECKS}>
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
                      <MenuItem key={option.label} onClick={() => handleMenuItemClick(option)}>
                        {option.label}
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
        )}
      </div>

      <CardImportDialog
        isOpen={state.showCardImportDialog}
        handleClose={closeImportCardWizard}
      ></CardImportDialog>
    </div>
  );
};

export default LeftNav;
