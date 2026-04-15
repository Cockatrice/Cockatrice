import { ReactElement } from 'react';
import Grid from '@mui/material/Grid';

import './ThreePaneLayout.css';

// @DEPRECATED
// This component sucks balls, dont use it. It will be removed sooner than later.
function ThreePaneLayout(props: ThreePaneLayoutProps) {
  return (
    <div className="three-pane-layout">
      <Grid container rowSpacing={0} columnSpacing={2} className="grid">
        <Grid size={{ xs: 12, md: 9, lg: 10 }} className="grid-main">
          <Grid className={
            'grid-main__top'
            + (props.fixedHeight ? ' fixedHeight' : '')
          }>
            {props.top}
          </Grid>
          <Grid className={
            'grid-main__bottom'
            + (props.fixedHeight ? ' fixedHeight' : '')
          }>
            {props.bottom}
          </Grid>
        </Grid>
        <Grid size={{ md: 3, lg: 2 }} sx={{ display: { xs: 'none', md: 'block' } }} className="grid-side">
          {props.side}
        </Grid>
      </Grid>
    </div>
  );
}

interface ThreePaneLayoutProps {
  top: ReactElement,
  bottom: ReactElement,
  side?: ReactElement,
  fixedHeight?: boolean,
}

export default ThreePaneLayout;
