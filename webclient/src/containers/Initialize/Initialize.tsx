import { useState } from 'react';
import { styled } from '@mui/material/styles';
import { useTranslation, Trans } from 'react-i18next';
import { connect } from 'react-redux';
import { Navigate } from 'react-router-dom';
import Typography from '@mui/material/Typography';

import { Images } from 'images';
import { ServerSelectors } from 'store';
import { RouteEnum } from 'types';
import Layout from 'containers/Layout/Layout';

import './Initialize.css';

const PREFIX = 'Initialize';

const classes = {
  root: `${PREFIX}-root`
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .Initialize-graphics': {
      color: theme.palette.primary.contrastText,
    },

    '& .Initialize-graphics__bar': {
      backgroundColor: theme.palette.primary.contrastText,
    },
  }
}));

const Initialize = ({ initialized }: InitializeProps) => {
  const { t } = useTranslation();

  return initialized
    ? <Navigate to={RouteEnum.LOGIN} />
    : (
      <Layout>
        <Root className={'Initialize ' + classes.root}>
          <div className='Initialize-content'>
            <img src={Images.Logo} alt="logo" />
            <Typography variant="subtitle1" className='subtitle'>{ t('InitializeContainer.title') }</Typography>
            <Trans i18nKey="InitializeContainer.subtitle">
              <Typography variant="subtitle2"></Typography>
              <Typography variant="subtitle2"></Typography>
            </Trans>
          </div>

          <div className="Initialize-graphics">
            <div className="topLeft Initialize-graphics__square" />
            <div className="topRight Initialize-graphics__square" />
            <div className="bottomRight Initialize-graphics__square" />
            <div className="bottomLeft Initialize-graphics__square" />
            <div className="topBar Initialize-graphics__bar" />
            <div className="bottomBar Initialize-graphics__bar" />
          </div>
        </Root>
      </Layout>
    );
}

interface InitializeProps {
  initialized: boolean;
}

const mapStateToProps = state => ({
  initialized: ServerSelectors.getInitialized(state),
});

export default connect(mapStateToProps)(Initialize);
