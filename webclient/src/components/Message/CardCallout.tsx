// eslint-disable-next-line
import React, { useMemo, useState } from 'react';
import { makeStyles } from '@material-ui/core/styles';
import Popover from '@material-ui/core/Popover';

import { CardDTO } from 'services';

import CardDetails from '../CardDetails/CardDetails';

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
  const [card, setCard] = useState(null);
  const [anchorEl, setAnchorEl] = React.useState(null);

  useMemo(() => {
    CardDTO.get(name).then(card => setCard(card));
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
      >{card?.name || name}</span>

      {
        card && (
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
              <CardDetails card={card} />
            </div>
          </Popover>
        )
      }
    </span>
  );
};

export default CardCallout;
