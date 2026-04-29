import { styled } from '@mui/material/styles';
import Popover from '@mui/material/Popover';

import CardDetails from '../CardDetails/CardDetails';
import TokenDetails from '../TokenDetails/TokenDetails';

import { useCardCallout } from './useCardCallout';

import './CardCallout.css';

const PREFIX = 'CardCallout';

const classes = {
  popover: `${PREFIX}-popover`,
  popoverContent: `${PREFIX}-popoverContent`
};

const Root = styled('span')(() => ({
  [`& .${classes.popover}`]: {
    pointerEvents: 'none',
  },

  [`& .${classes.popoverContent}`]: {
    pointerEvents: 'none',
  }
}));

interface CardCalloutProps {
  name: string;
}

const CardCallout = ({ name }: CardCalloutProps) => {
  const { card, token, anchorEl, open, handlePopoverOpen, handlePopoverClose } =
    useCardCallout(name);

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
              {card && (<CardDetails card={card} />)}
              {token && (<TokenDetails token={token} />)}
            </div>
          </Popover>
        )
      }
    </Root>
  );
};

export default CardCallout;
