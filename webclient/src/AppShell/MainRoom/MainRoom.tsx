// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';

import './MainRoom.css';

class MainRoom extends Component<MainRoomProps, MainRoomState> {
  render() {
    return (
      <div dangerouslySetInnerHTML={{ __html: this.props.message }}></div>
    )
  }
}

export interface MainRoomState {
  message: string;
}

export interface MainRoomProps extends MainRoomState {

}

const mapStateToProps = state => ({
  message: state.server.message
});

export default connect(mapStateToProps, {})(MainRoom);