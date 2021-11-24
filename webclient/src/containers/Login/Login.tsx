// eslint-disable-next-line
import React, { useState, useCallback } from "react";
import { connect } from 'react-redux';
import { Redirect } from 'react-router-dom';
import { makeStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';


import { AuthenticationService } from 'api';
import { RequestPasswordResetDialog, ResetPasswordDialog } from 'components';
import { LoginForm } from 'forms';
import { useReduxEffect } from 'hooks';
import { Images } from 'images';
import { HostDTO } from 'services';
import { RouteEnum, WebSocketConnectOptions, getHostPort } from 'types';
import { ServerSelectors, ServerTypes } from 'store';

import './Login.css';

const useStyles = makeStyles(theme => ({
  root: {
    '& .login-content__header': {
      color: theme.palette.success.light
    },

    '& .login-content__description': {
      backgroundColor: theme.palette.primary.main,
      color: theme.palette.primary.contrastText,
    },

    '& .login-content__description-bar': {
      backgroundColor: theme.palette.primary.dark,
    },

    '& .login-content__description-cards__card': {
      backgroundColor: theme.palette.background.paper,
      color: theme.palette.primary.main,
    },

    [theme.breakpoints.up('lg')]: {
      '& .login-content': {
        maxWidth: '1000px',
      },

      '& .login-content__form': {
        width: '50%',
      },

      '& .login-content__description': {
        width: '50%',
        display: 'flex',
      },
    },
  },
}));

const Login = ({ state, description }: LoginProps) => {
  const classes = useStyles();
  const isConnected = AuthenticationService.isConnected(state);

  const [hostIdToRemember, setHostIdToRemember] = useState(null);
  const [dialogState, setDialogState] = useState({
    passwordResetRequestDialog: false,
    resetPasswordDialog: false
  });

  useReduxEffect(() => {
    closeRequestPasswordResetDialog();
    openResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_REQUESTED, []);

  useReduxEffect(() => {
    closeResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_SUCCESS, []);

  useReduxEffect(({ options: { hashedPassword } }) => {
    if (hostIdToRemember) {
      HostDTO.get(hostIdToRemember).then(host => {
        host.hashedPassword = hashedPassword;
        host.save();
      });
    }
  }, ServerTypes.LOGIN_SUCCESSFUL, [hostIdToRemember]);

  const showDescription = () => {
    return !isConnected && description?.length;
  };

  function createAccount() {
    console.log('Login.createAccount->openForgotPasswordDialog');
  }

  const onSubmit = useCallback((loginForm) => {
    const {
      userName,
      password,
      selectedHost,
      selectedHost: {
        id: hostId,
        hashedPassword
      },
      remember
    } = loginForm;

    const options: WebSocketConnectOptions = {
      ...getHostPort(selectedHost),
      userName,
      password,
      hashedPassword
    };

    updateHost(loginForm);

    if (remember) {
      setHostIdToRemember(hostId);
    }

    AuthenticationService.login(options as WebSocketConnectOptions);
  }, []);

  function updateHost({ selectedHost, userName, hashedPassword, remember }) {
    HostDTO.get(selectedHost.id).then(hostDTO => {
      hostDTO.remember = remember;
      hostDTO.userName = remember ? userName : null;
      hostDTO.hashedPassword = remember ? hashedPassword : null;

      hostDTO.save();
    });
  }

  const handleRequestPasswordResetDialogSubmit = async ({ user, email, host, port }) => {
    if (email) {
      AuthenticationService.resetPasswordChallenge({ user, email, host, port } as any);
    } else {
      AuthenticationService.resetPasswordRequest({ user, host, port } as any);
    }
  };

  const handleResetPasswordDialogSubmit = async ({ user, token, newPassword, passwordAgain, host, port }) => {
    AuthenticationService.resetPassword({ user, token, newPassword, host, port } as any);
  };

  const closeRequestPasswordResetDialog = () => {
    setDialogState(s => ({ ...s, passwordResetRequestDialog: false }));
  }

  const openRequestPasswordResetDialog = () => {
    setDialogState(s => ({ ...s, passwordResetRequestDialog: true }));
  }

  const closeResetPasswordDialog = () => {
    setDialogState(s => ({ ...s, resetPasswordDialog: false }));
  }

  const openResetPasswordDialog = () => {
    setDialogState(s => ({ ...s, resetPasswordDialog: true }));
  }

  return (
    <div className={'login overflow-scroll ' + classes.root}>
      { isConnected && <Redirect from="*" to={RouteEnum.SERVER} />}

      <div className="login__wrapper">
        <Paper className="login-content">
          <div className="login-content__form">
            <div className="login-content__header">
              <img src={Images.Logo} alt="logo" />
              <span>COCKATRICE</span>
            </div>
            <Typography variant="h1">Login</Typography>
            <Typography variant="subtitle1">A cross-platform virtual tabletop for multiplayer card games.</Typography>
            <div className="login-form">
              <LoginForm onSubmit={onSubmit} />
            </div>

            {
              showDescription() && (
                <Paper className="login-content__connectionStatus">
                  {description}
                </Paper>
              )
            }

            <div className="login-footer">
              <div className="login-footer_register">
                <span>Not registered yet?</span>
                <Button color="primary" onClick={createAccount}>Create an account</Button>
              </div>
              <Typography variant="subtitle2" className="login-footer__copyright">
                Cockatrice is an open source project. { new Date().getUTCFullYear() }
              </Typography>
            </div>
          </div>
          <div className="login-content__description">
            <div className="login-content__description-graphics">
              <div className="topLeft login-content__description-square" />
              <div className="topRight login-content__description-square" />
              <div className="bottomRight login-content__description-square" />
              <div className="bottomLeft login-content__description-square" />
              <div className="topBar login-content__description-bar" />
              <div className="bottomBar login-content__description-bar" />
            </div>
            <div className="login-content__description-wrapper">
              <div className="login-content__description-cards">
                <div className="login-content__description-cards__card leftCard">
                  <div className="login-content__description-cards__card-wrapper">
                    <img src={Images.Faces.face1} alt='Stock Player' />
                    <span>1mrlee</span>
                  </div>
                </div>
                <div className="login-content__description-cards__card rightCard">
                  <div className="login-content__description-cards__card-wrapper">
                    <img src={Images.Faces.face2} alt='Stock Player' />
                    <span>CyberX</span>
                  </div>
                </div>
                <div className="login-content__description-cards__card topCard">
                  <div className="login-content__description-cards__card-wrapper">
                    <img src={Images.Faces.face3} alt='Stock Player' />
                    <span>Gamer69</span>
                  </div>
                </div>
              </div>
              { /*<img src={loginGraphic} className="login-content__description-image"/>*/}
              <p className="login-content__description-subtitle1">Play multiplayer card games online.</p>
              <p className="login-content__description-subtitle2">
                Cross-platform virtual tabletop for multiplayer card games. Forever free.
              </p>
            </div>
          </div>
        </Paper>
      </div>

      <RequestPasswordResetDialog
        isOpen={dialogState.passwordResetRequestDialog}
        onSubmit={handleRequestPasswordResetDialogSubmit}
        handleClose={closeRequestPasswordResetDialog}
      />

      <ResetPasswordDialog
        isOpen={dialogState.resetPasswordDialog}
        onSubmit={handleResetPasswordDialogSubmit}
        handleClose={closeResetPasswordDialog}
      />
    </div>
  );
}

interface LoginProps {
  state: number;
  description: string;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  description: ServerSelectors.getDescription(state),
});

export default connect(mapStateToProps)(Login);
