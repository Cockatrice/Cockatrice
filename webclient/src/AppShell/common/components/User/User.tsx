// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';

import { User as UserType } from '../../types';

import './User.css';

class User extends Component<UserProps> {

  render() {
    const { name } = this.props.user;
    
    return (
      <div>{name}</div>
    );
  }
}

interface UserProps {
  user: UserType
}

const mapStateToProps = (state) => ({

});

export default connect(mapStateToProps)(User);
