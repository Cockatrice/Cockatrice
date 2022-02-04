import { useState } from 'react';
import { connect } from 'react-redux';
import { Redirect, withRouter } from 'react-router-dom';
import { makeStyles } from '@material-ui/core/styles';
import Typography from '@material-ui/core/Typography';

import { Images } from 'images';
import { ServerSelectors } from 'store';
import { RouteEnum } from 'types';

import './Initialize.css';

const useStyles = makeStyles(theme => ({
  root: {
    '& .Initialize-graphics': {
      color: theme.palette.primary.contrastText,
    },

    '& .Initialize-graphics__bar': {
      backgroundColor: theme.palette.primary.contrastText,
    },
  },
}));

const Initialize = ({ initialized }: InitializeProps) => {
  const classes = useStyles();

  return false
    ? <Redirect from="*" to={RouteEnum.LOGIN} />
    : (
      <div className={'Initialize ' + classes.root}>
        <div className='Initialize-content'>
          <img src={Images.Logo} alt="logo" />
          <Typography variant="subtitle1" className='subtitle'>DID YOU KNOW</Typography>
          <Typography variant="subtitle2">Cockatrice is run by volunteers</Typography>
          <Typography variant="subtitle2">that love card games!</Typography>
        </div>

        <div className="Initialize-graphics">
          <div className="topLeft Initialize-graphics__square" />
          <div className="topRight Initialize-graphics__square" />
          <div className="bottomRight Initialize-graphics__square" />
          <div className="bottomLeft Initialize-graphics__square" />
          <div className="topBar Initialize-graphics__bar" />
          <div className="bottomBar Initialize-graphics__bar" />
        </div>
      </div>
    );
}

interface InitializeProps {
  initialized: boolean;
}

const mapStateToProps = state => ({
  initialized: ServerSelectors.getInitialized(state),
});

export default withRouter(connect(mapStateToProps)(Initialize));
