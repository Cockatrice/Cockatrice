// eslint-disable-next-line
import React, { useMemo, useState } from 'react';
import { styled } from '@mui/material/styles';
import Popover from '@mui/material/Popover';

import { CardDTO, TokenDTO } from 'services';

import CardDetails from '../CardDetails/CardDetails';
import TokenDetails from '../TokenDetails/TokenDetails';

import './CardCallout.css';

const PREFIX = 'CardCallout';

const classes = {
  popover: `${PREFIX}-popover`,
  popoverContent: `${PREFIX}-popoverContent`
};

const Root = styled('span')(({ theme }) => ({
  [`& .${classes.popover}`]: {
    pointerEvents: 'none',
  },

  [`& .${classes.popoverContent}`]: {
    pointerEvents: 'none',
  }
}));

const CardCallout = ({ name }) => {
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
    <Root className='callout'>
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
    </Root>
  );
};

export default CardCallout;
