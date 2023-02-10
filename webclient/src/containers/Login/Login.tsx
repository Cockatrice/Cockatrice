import { useState, useCallback } from 'react';
import { styled } from '@mui/material/styles';
import { useTranslation } from 'react-i18next';
import { connect } from 'react-redux';
import { Navigate } from 'react-router-dom';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import { AuthenticationService } from 'api';
import { RegistrationDialog, RequestPasswordResetDialog, ResetPasswordDialog, AccountActivationDialog } from 'dialogs';
import { LanguageDropdown } from 'components';
import { LoginForm } from 'forms';
import { useReduxEffect, useFireOnce } from 'hooks';
import { Images } from 'images';
import { HostDTO, serverProps } from 'services';
import { RouteEnum, WebSocketConnectOptions, getHostPort } from 'types';
import { ServerSelectors, ServerTypes } from 'store';
import Layout from 'containers/Layout/Layout';

import './Login.css';
import { useToast } from 'components/Toast';

const PREFIX = 'Login';

const classes = {
  root: `${PREFIX}-root`
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
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
  }
}));

const Login = ({ state, description, connectOptions }: LoginProps) => {
  const { t } = useTranslation();

  const isConnected = AuthenticationService.isConnected(state);

  const [rememberLogin, setRememberLogin] = useState(null);
  const [dialogState, setDialogState] = useState({
    passwordResetRequestDialog: false,
    resetPasswordDialog: false,
    registrationDialog: false,
    activationDialog: false,
  });
  const [userToResetPassword, setUserToResetPassword] = useState(null);

  const passwordResetToast = useToast({ key: 'password-reset-success', children: t('LoginContainer.toasts.passwordResetSuccess') });
  const accountActivatedToast = useToast({
    key: 'account-activation-success',
    children: t('LoginContainer.toasts.accountActivationSuccess')
  });

  useReduxEffect(() => {
    closeRequestPasswordResetDialog();
    openResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_REQUESTED, []);

  useReduxEffect(() => {
    passwordResetToast.openToast()
    closeResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_SUCCESS, []);

  useReduxEffect(() => {
    accountActivatedToast.openToast()
    closeActivateAccountDialog();
  }, ServerTypes.ACCOUNT_ACTIVATION_SUCCESS, []);

  useReduxEffect(() => {
    closeRegistrationDialog();
    openActivateAccountDialog();
  }, ServerTypes.ACCOUNT_AWAITING_ACTIVATION, []);

  useReduxEffect(() => {
    resetSubmitButton();
  }, [ServerTypes.CONNECTION_FAILED, ServerTypes.LOGIN_FAILED], []);

  useReduxEffect(({ options: { hashedPassword } }) => {
    updateHost(hashedPassword, rememberLogin);
  }, ServerTypes.LOGIN_SUCCESSFUL, [rememberLogin]);

  const showDescription = () => {
    return !isConnected && description?.length;
  };

  const onSubmitLogin = useCallback((loginForm) => {
    setRememberLogin(loginForm);
    const { userName, password, selectedHost, remember } = loginForm;

    const options: WebSocketConnectOptions = {
      ...getHostPort(selectedHost),
      userName,
      password
    };

    if (remember && !password) {
      options.hashedPassword = selectedHost.hashedPassword;
    }

    AuthenticationService.login(options as WebSocketConnectOptions);
  }, []);

  const [submitButtonDisabled, resetSubmitButton, handleLogin] = useFireOnce(onSubmitLogin);

  const updateHost = (hashedPassword, { selectedHost, remember, userName }) => {
    HostDTO.get(selectedHost.id).then(hostDTO => {
      hostDTO.remember = remember;
      hostDTO.userName = remember ? userName : null;
      hostDTO.hashedPassword = remember ? hashedPassword : null;

      hostDTO.save();
    });
  };

  const handleRegistrationDialogSubmit = (registerForm) => {
    setRememberLogin(registerForm);
    const { userName, password, email, country, realName, selectedHost } = registerForm;

    AuthenticationService.register({
      ...getHostPort(selectedHost),
      userName,
      password,
      email,
      country,
      realName,
    });
  };

  const handleAccountActivationDialogSubmit = ({ token }) => {
    AuthenticationService.activateAccount({
      ...connectOptions,
      token,
    });
  };

  const handleRequestPasswordResetDialogSubmit = (form) => {
    const { userName, email, selectedHost } = form;
    const { host, port } = getHostPort(selectedHost);

    if (email) {
      AuthenticationService.resetPasswordChallenge({ userName, email, host, port });
    } else {
      setUserToResetPassword(userName);
      AuthenticationService.resetPasswordRequest({ userName, host, port });
    }
  };

  const handleResetPasswordDialogSubmit = ({ userName, token, newPassword, selectedHost }) => {
    const { host, port } = getHostPort(selectedHost);

    AuthenticationService.resetPassword({ userName, token, newPassword, host, port });
  };

  const skipTokenRequest = (userName) => {
    setUserToResetPassword(userName);

    setDialogState(s => ({ ...s,
      passwordResetRequestDialog: false,
      resetPasswordDialog: true,
    }));
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

  const closeRegistrationDialog = () => {
    setDialogState(s => ({ ...s, registrationDialog: false }));
  }

  const openRegistrationDialog = () => {
    setDialogState(s => ({ ...s, registrationDialog: true }));
  }

  const closeActivateAccountDialog = () => {
    setDialogState(s => ({ ...s, activationDialog: false }));
  };

  const openActivateAccountDialog = () => {
    setDialogState(s => ({ ...s, activationDialog: true }));
  };

  return (
    <Layout showNav={false} noHeightLimit={true}>
      <Root className={'login overflow-scroll ' + classes.root}>
        { isConnected && <Navigate to={RouteEnum.SERVER} />}

        <div className="login__wrapper">
          <Paper className="login-content">
            <div className="login-content__form">
              <div className="login-content__header">
                <img src={Images.Logo} alt="logo" />
                <span>COCKATRICE</span>
              </div>
              <Typography variant="h1">{ t('LoginContainer.header.title') }</Typography>
              <Typography variant="subtitle1">{ t('LoginContainer.header.subtitle') }</Typography>
              <div className="login-form">
                <LoginForm
                  onSubmit={handleLogin}
                  onResetPassword={openRequestPasswordResetDialog}
                  disableSubmitButton={submitButtonDisabled}
                />
              </div>

              {
                showDescription() && (
                  <Paper className="login-content__connectionStatus">
                    {description}
                  </Paper>
                )
              }

              <div className="login-footer">
                <div className="login-footer__register">
                  <span>{ t('LoginContainer.footer.registerPrompt') }</span>
                  <Button color="primary" onClick={openRegistrationDialog}>{ t('LoginContainer.footer.registerAction') }</Button>
                </div>
                <Typography variant="subtitle2">
                  { t('LoginContainer.footer.credit') } - { new Date().getUTCFullYear() }
                </Typography>

                {
                  serverProps.REACT_APP_VERSION && (
                    <Typography variant="subtitle2">
                      { t('LoginContainer.footer.version') }: { serverProps.REACT_APP_VERSION }
                    </Typography>
                  )
                }

                <div className="login-footer__language">
                  <LanguageDropdown />
                </div>
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
                <p className="login-content__description-subtitle1">{ t('LoginContainer.content.subtitle1') }</p>
                <p className="login-content__description-subtitle2">{ t('LoginContainer.content.subtitle2') }</p>
              </div>
            </div>
          </Paper>
        </div>

        <RegistrationDialog
          isOpen={dialogState.registrationDialog}
          onSubmit={handleRegistrationDialogSubmit}
          handleClose={closeRegistrationDialog}
        />

        <RequestPasswordResetDialog
          isOpen={dialogState.passwordResetRequestDialog}
          onSubmit={handleRequestPasswordResetDialogSubmit}
          handleClose={closeRequestPasswordResetDialog}
          skipTokenRequest={skipTokenRequest}
        />

        <ResetPasswordDialog
          isOpen={dialogState.resetPasswordDialog}
          onSubmit={handleResetPasswordDialogSubmit}
          handleClose={closeResetPasswordDialog}
          userName={userToResetPassword}
        />

        <AccountActivationDialog
          isOpen={dialogState.activationDialog}
          onSubmit={handleAccountActivationDialogSubmit}
          handleClose={closeActivateAccountDialog}
        />
      </Root>
    </Layout>
  );
}

interface LoginProps {
  state: number;
  description: string;
  connectOptions: WebSocketConnectOptions;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  description: ServerSelectors.getDescription(state),
  connectOptions: ServerSelectors.getConnectOptions(state),
});

export default connect(mapStateToProps)(Login);
