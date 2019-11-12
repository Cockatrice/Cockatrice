// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';

import { User } from 'types';

import './UserDisplay.css';

class UserDisplay extends Component<UserDisplayProps> {

  render() {
    const { name } = this.props.user;
    
    return (
      <div>{name}</div>
    );
  }
}

interface UserDisplayProps {
  user: User
}

const mapStateToProps = (state) => ({

});

export default connect(mapStateToProps)(UserDisplay);
