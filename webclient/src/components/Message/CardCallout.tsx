// eslint-disable-next-line
import React, { useMemo, useState } from 'react';
import { makeStyles } from '@material-ui/core/styles';
import Popover from '@material-ui/core/Popover';

import { CardDTO, TokenDTO } from 'services';

import CardDetails from '../CardDetails/CardDetails';
import TokenDetails from '../TokenDetails/TokenDetails';

import './CardCallout.css';

const useStyles = makeStyles(theme => ({
  popover: {
    pointerEvents: 'none',
  },
  popoverContent: {
    pointerEvents: 'none',
  },
}));

const CardCallout = ({ name }) => {
  const classes = useStyles();
  const [card, setCard] = useState<CardDTO>(null);
  const [token, setToken] = useState<TokenDTO>(null);
  const [anchorEl, setAnchorEl] = useState<Element>(null);

  useMemo(async () => {
    const card = await CardDTO.get(name);
    if (card) {
      return setCard(card)
    }

    const token = await TokenDTO.get(name);
    if (token) {
      return setToken(token);
    }
  }, [name]);

  const handlePopoverOpen = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handlePopoverClose = () => {
    setAnchorEl(null);
  };

  const open = Boolean(anchorEl);

  return (
    <span className='callout'>
      <span
        onMouseEnter={handlePopoverOpen}
        onMouseLeave={handlePopoverClose}
      >{card?.name || token?.name?.value || name}</span>

      {
        (card || token) && (
          <Popover
            open={open}
            anchorEl={anchorEl}
            onClose={handlePopoverClose}
            className={classes.popover}
            classes={{
              paper: classes.popoverContent,
            }}
            anchorOrigin={{
              vertical: 'top',
              horizontal: 'right',
            }}
            transformOrigin={{
              vertical: 'bottom',
              horizontal: 'left',
            }}
          >
            <div className="callout-card">
              { card && (<CardDetails card={card} />) }
              { token && (<TokenDetails token={token} />) }
            </div>
          </Popover>
        )
      }
    </span>
  );
};

export default CardCallout;
