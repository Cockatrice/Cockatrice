import { Component, CElement } from 'react';
import Grid from '@mui/material/Grid';

import './ThreePaneLayout.css';

// @DEPRECATED
// This component sucks balls, dont use it. It will be removed sooner than later.
class ThreePaneLayout extends Component<ThreePaneLayoutProps> {
  render() {
    return (
      <div className="three-pane-layout">
        <Grid container rowSpacing={0} columnSpacing={2} className="grid">
          <Grid size={{ xs: 12, md: 9, lg: 10 }} className="grid-main">
            <Grid className={
              'grid-main__top'
              + (this.props.fixedHeight ? ' fixedHeight' : '')
            }>
              {this.props.top}
            </Grid>
            <Grid className={
              'grid-main__bottom'
              + (this.props.fixedHeight ? ' fixedHeight' : '')
            }>
              {this.props.bottom}
            </Grid>
          </Grid>
          <Grid size={{ md: 3, lg: 2 }} sx={{ display: { xs: 'none', md: 'block' } }} className="grid-side">
            {this.props.side}
          </Grid>
        </Grid>
      </div>
    );
  }
}

interface ThreePaneLayoutProps {
  top: CElement<any, any>,
  bottom: CElement<any, any>,
  side?: CElement<any, any>,
  fixedHeight?: boolean,
}

export default ThreePaneLayout;
