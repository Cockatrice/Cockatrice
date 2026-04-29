import { styled } from '@mui/material/styles';
import { useTranslation } from 'react-i18next';
import { Navigate } from 'react-router-dom';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import { RegistrationDialog, RequestPasswordResetDialog, ResetPasswordDialog, AccountActivationDialog } from '@app/dialogs';
import { LanguageDropdown } from '@app/components';
import { LoginForm } from '@app/forms';
import { Images } from '@app/images';
import { serverProps } from '@app/services';
import { App } from '@app/types';
import Layout from '../Layout/Layout';

import { useLogin } from './useLogin';

import './Login.css';

const PREFIX = 'Login';

const classes = {
  root: `${PREFIX}-root`,
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .login-content__header': {
      color: theme.palette.success.light,
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

const Login = () => {
  const { t } = useTranslation();
  const {
    description,
    isConnected,
    dialogState,
    userToResetPassword,
    submitButtonDisabled,
    handleLogin,
    showDescription,
    handleRegistrationDialogSubmit,
    handleAccountActivationDialogSubmit,
    handleRequestPasswordResetDialogSubmit,
    handleResetPasswordDialogSubmit,
    skipTokenRequest,
    closeRequestPasswordResetDialog,
    openRequestPasswordResetDialog,
    closeResetPasswordDialog,
    closeRegistrationDialog,
    openRegistrationDialog,
    closeActivateAccountDialog,
  } = useLogin();

  return (
    <Layout showNav={false} noHeightLimit={true}>
      <Root className={'login overflow-scroll ' + classes.root}>
        {isConnected && <Navigate to={App.RouteEnum.SERVER} />}

        <div className="login__wrapper">
          <Paper className="login-content">
            <div className="login-content__form">
              <div className="login-content__header">
                <img src={Images.Logo} alt="logo" />
                <span>COCKATRICE</span>
              </div>
              <Typography variant="h1">{t('LoginContainer.header.title')}</Typography>
              <Typography variant="subtitle1">{t('LoginContainer.header.subtitle')}</Typography>
              <div className="login-form">
                <LoginForm
                  onSubmit={handleLogin}
                  onResetPassword={openRequestPasswordResetDialog}
                  disableSubmitButton={submitButtonDisabled}
                />
              </div>

              {showDescription() && (
                <Paper className="login-content__connectionStatus">
                  {description}
                </Paper>
              )}

              <div className="login-footer">
                <div className="login-footer__register">
                  <span>{t('LoginContainer.footer.registerPrompt')}</span>
                  <Button color="primary" onClick={openRegistrationDialog}>{t('LoginContainer.footer.registerAction')}</Button>
                </div>
                <Typography variant="subtitle2">
                  {t('LoginContainer.footer.credit')} - {new Date().getUTCFullYear()}
                </Typography>

                {serverProps.REACT_APP_VERSION && (
                  <Typography variant="subtitle2">
                    {t('LoginContainer.footer.version')}: {serverProps.REACT_APP_VERSION}
                  </Typography>
                )}

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
                <p className="login-content__description-subtitle1">{t('LoginContainer.content.subtitle1')}</p>
                <p className="login-content__description-subtitle2">{t('LoginContainer.content.subtitle2')}</p>
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
};

export default Login;
