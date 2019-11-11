// eslint-disable-next-line
import React, { Component, CElement } from 'react';
import { connect } from 'react-redux';

import './ThreePaneLayout.css';

class ThreePaneLayout extends Component<ThreePaneLayoutProps> {
  render() {
    return (
      <div className="three-pane-layout">
        <div className="three-pane-layout__main">
          <div className="three-pane-layout__main-top overflow-scroll">
            {this.props.top}
          </div>
          <div className="three-pane-layout__main-bottom overflow-scroll">
            {this.props.bottom}
          </div>
        </div>
        <div className="three-pane-layout__side overflow-scroll">
          {this.props.side}
        </div>
      </div>
    );
  }
}

interface ThreePaneLayoutProps {
  top: CElement<any, any>,
  bottom: CElement<any, any>,
  side?: CElement<any, any>
}

const mapStateToProps = state => ({

});

export default connect(mapStateToProps)(ThreePaneLayout);
