// eslint-disable-next-line
import React, { Component, CElement } from 'react';
import { connect } from 'react-redux';
import Grid from '@material-ui/core/Grid';
import Hidden from '@material-ui/core/Hidden';

import './ThreePaneLayout.css';

class ThreePaneLayout extends Component<ThreePaneLayoutProps> {
  render() {
    return (
      <div className="three-pane-layout">
        <Grid container spacing={2} className="grid">
          <Grid item xs={12} md={9} lg={10} className="grid-main">
            <Grid item className={
              "grid-main__top"
              + (this.props.fixedHeight ? ' fixedHeight' : '')
            }>
              <div className="grid-content overflow-scroll">
                {this.props.top}
              </div>
            </Grid>
            <Grid item className="grid-main__bottom">
              <div className="grid-content overflow-scroll">
                {this.props.bottom}
              </div>
            </Grid>
          </Grid>
          <Hidden smDown>
            <Grid item md={3} lg={2} className="grid-side">
              <div className="grid-content overflow-scroll">
                {this.props.side}
              </div>
            </Grid>
          </Hidden>
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

const mapStateToProps = state => ({

});

export default connect(mapStateToProps)(ThreePaneLayout);
