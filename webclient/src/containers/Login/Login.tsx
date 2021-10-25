// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Redirect } from "react-router-dom";
import { styled } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';

import { AuthenticationService } from "api";
import { LoginForm } from "forms";
import { RouteEnum, StatusEnum } from "types";
import { /* ServerDispatch, */ ServerSelectors } from "store";

import "./Login.css";
import logo from "images/logo.png";

const Login = ({ state, description }: LoginProps) => {
  const isConnected = AuthenticationService.isConnected(state);

  const showDescription = () => {
    return !isConnected && description?.length;
  }

  const createAccount = () => {
    console.log('Login.createAccount->ServerDispatch.openForgotPasswordDialog');
    // ServerDispatch.openCreateAccountDialog();
  };

  return (
    <div className="login overflow-scroll">
      { isConnected && <Redirect from="*" to={RouteEnum.SERVER} />}

      <div className="login__wrapper">
        <ThemedLoginContent className="login-content">
          <div className="login-content__form">
            <ThemedLoginHeader className="login-content__header">
              <img src={logo} alt="logo" />
              <span>COCKATRICE</span>
            </ThemedLoginHeader>
            <Typography variant="h1">Login</Typography>
            <Typography variant="subtitle1">A cross-platform virtual tabletop for multiplayer card games.</Typography>
            <div className="login-form">
              <LoginForm onSubmit={AuthenticationService.connect} />
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
                Cockatrice is an open source project @{ new Date().getUTCFullYear() }
              </Typography>
            </div>
          </div>

          <ThemedLoginDescription className="login-content__description">
            description
          </ThemedLoginDescription>
        </ThemedLoginContent>
      </div>
    </div>
  );
}

const ThemedLoginContent = styled('div')(({ theme }) => ({
  backgroundColor: theme.palette.background.paper
}));

const ThemedLoginDescription = styled('div')(({ theme }) => ({
  backgroundColor: theme.palette.primary.main,
  color: theme.palette.primary.contrastText,
}));

const ThemedLoginHeader = styled('div')(({ theme }) => ({
  color: theme.palette.success.light
}));

interface LoginProps {
  state: number;
  description: string;
}

const mapStateToProps = state => ({
  state: ServerSelectors.getState(state),
  description: ServerSelectors.getDescription(state),
});

export default connect(mapStateToProps)(Login);
