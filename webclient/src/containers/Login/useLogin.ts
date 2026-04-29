import { useCallback, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { useToast } from '@app/components';
import type {
  LoginFormValues,
  RegisterFormValues,
  RequestPasswordResetFormValues,
  ResetPasswordFormValues,
} from '@app/forms';
import { useAutoLogin, useFireOnce, useKnownHosts, useReduxEffect, useWebClient } from '@app/hooks';
import { getHostPort } from '@app/services';
import { ServerSelectors, ServerTypes, useAppSelector } from '@app/store';
import { WebsocketTypes } from '@app/websocket/types';

export interface LoginDialogState {
  passwordResetRequestDialog: boolean;
  resetPasswordDialog: boolean;
  registrationDialog: boolean;
  activationDialog: boolean;
}

export interface Login {
  description: string | undefined;
  isConnected: boolean;
  dialogState: LoginDialogState;
  userToResetPassword: string | null;
  submitButtonDisabled: boolean;
  handleLogin: (form: LoginFormValues) => void;
  showDescription: () => boolean;
  handleRegistrationDialogSubmit: (form: RegisterFormValues) => void;
  handleAccountActivationDialogSubmit: (args: { token: string }) => void;
  handleRequestPasswordResetDialogSubmit: (form: RequestPasswordResetFormValues) => void;
  handleResetPasswordDialogSubmit: (form: ResetPasswordFormValues) => void;
  skipTokenRequest: (userName: string) => void;
  closeRequestPasswordResetDialog: () => void;
  openRequestPasswordResetDialog: () => void;
  closeResetPasswordDialog: () => void;
  closeRegistrationDialog: () => void;
  openRegistrationDialog: () => void;
  closeActivateAccountDialog: () => void;
}

export function useLogin(): Login {
  const description = useAppSelector((s) => ServerSelectors.getDescription(s));
  const isConnected = useAppSelector(ServerSelectors.getIsConnected);
  const connectionAttemptMade = useAppSelector(ServerSelectors.getConnectionAttemptMade);
  const webClient = useWebClient();
  const { t } = useTranslation();

  const [pendingActivationOptions, setPendingActivationOptions] =
    useState<WebsocketTypes.PendingActivationContext | null>(null);

  const rememberLoginRef = useRef<LoginFormValues | RegisterFormValues | null>(null);
  const knownHosts = useKnownHosts();
  const [dialogState, setDialogState] = useState<LoginDialogState>({
    passwordResetRequestDialog: false,
    resetPasswordDialog: false,
    registrationDialog: false,
    activationDialog: false,
  });
  const [userToResetPassword, setUserToResetPassword] = useState<string | null>(null);

  const passwordResetToast = useToast({
    key: 'password-reset-success',
    children: t('LoginContainer.toasts.passwordResetSuccess'),
  });
  const accountActivatedToast = useToast({
    key: 'account-activation-success',
    children: t('LoginContainer.toasts.accountActivationSuccess'),
  });

  const closeRequestPasswordResetDialog = () => {
    setDialogState((s) => ({ ...s, passwordResetRequestDialog: false }));
  };

  const openRequestPasswordResetDialog = () => {
    setDialogState((s) => ({ ...s, passwordResetRequestDialog: true }));
  };

  const closeResetPasswordDialog = () => {
    setDialogState((s) => ({ ...s, resetPasswordDialog: false }));
  };

  const openResetPasswordDialog = () => {
    setDialogState((s) => ({ ...s, resetPasswordDialog: true }));
  };

  const closeRegistrationDialog = () => {
    setDialogState((s) => ({ ...s, registrationDialog: false }));
  };

  const openRegistrationDialog = () => {
    setDialogState((s) => ({ ...s, registrationDialog: true }));
  };

  const closeActivateAccountDialog = () => {
    setDialogState((s) => ({ ...s, activationDialog: false }));
  };

  const openActivateAccountDialog = () => {
    setDialogState((s) => ({ ...s, activationDialog: true }));
  };

  useReduxEffect(() => {
    closeRequestPasswordResetDialog();
    openResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_REQUESTED, []);

  useReduxEffect(() => {
    passwordResetToast.openToast();
    closeResetPasswordDialog();
  }, ServerTypes.RESET_PASSWORD_SUCCESS, []);

  useReduxEffect(() => {
    accountActivatedToast.openToast();
    closeActivateAccountDialog();
    setPendingActivationOptions(null);
  }, ServerTypes.ACCOUNT_ACTIVATION_SUCCESS, []);

  useReduxEffect<{ options: WebsocketTypes.PendingActivationContext }>(({ payload: { options } }) => {
    setPendingActivationOptions(options);
    closeRegistrationDialog();
    openActivateAccountDialog();
  }, ServerTypes.ACCOUNT_AWAITING_ACTIVATION, []);

  const onSubmitLogin = useCallback((loginForm: LoginFormValues) => {
    rememberLoginRef.current = loginForm;
    const { userName, password, selectedHost, remember } = loginForm;

    const options: Omit<WebsocketTypes.LoginConnectOptions, 'reason'> = {
      ...getHostPort(selectedHost),
      userName,
      password,
    };

    if (remember && !password) {
      options.hashedPassword = selectedHost.hashedPassword;
    }

    webClient.request.authentication.login(options);
  }, [webClient]);

  const [submitButtonDisabled, resetSubmitButton, handleLogin] = useFireOnce(onSubmitLogin);

  useReduxEffect(() => {
    resetSubmitButton();
  }, [ServerTypes.CONNECTION_FAILED, ServerTypes.LOGIN_FAILED], []);

  const updateHost = (
    hashedPassword: string | undefined,
    { selectedHost, remember, userName }: LoginFormValues,
  ) => {
    if (selectedHost.id == null) {
      return;
    }
    // Only honour the Remember checkbox when we actually received a hash to
    // store. A server that advertises SupportsPasswordHash but returns an empty
    // salt falls through to a plain-password login (no hash produced); writing
    // `remember: true` with a null hash would leave the checkbox visibly
    // checked on next load while the stored-password flow silently couldn't
    // activate. Resetting to the unchecked state makes the failure visible.
    const persistCredentials = remember && Boolean(hashedPassword);
    knownHosts.update(selectedHost.id, {
      remember: persistCredentials,
      userName: persistCredentials ? userName : null,
      hashedPassword: persistCredentials ? hashedPassword : null,
    });
  };

  useReduxEffect<{ options: WebsocketTypes.LoginSuccessContext }>(({ payload: { options } }) => {
    const loginForm = rememberLoginRef.current;
    if (loginForm && 'remember' in loginForm) {
      updateHost(options.hashedPassword, loginForm);
    }
  }, ServerTypes.LOGIN_SUCCESSFUL, []);

  useAutoLogin(handleLogin, connectionAttemptMade);

  const showDescription = () => {
    return Boolean(!isConnected && description?.length);
  };

  const handleRegistrationDialogSubmit = (registerForm: RegisterFormValues) => {
    rememberLoginRef.current = registerForm;
    const { userName, password, email, country, realName, selectedHost } = registerForm;

    webClient.request.authentication.register({
      ...getHostPort(selectedHost),
      userName,
      password,
      email,
      country,
      realName,
    });
  };

  const handleAccountActivationDialogSubmit = ({ token }: { token: string }) => {
    if (!pendingActivationOptions) {
      return;
    }
    webClient.request.authentication.activateAccount({
      host: pendingActivationOptions.host,
      port: pendingActivationOptions.port,
      userName: pendingActivationOptions.userName,
      token,
    });
  };

  const handleRequestPasswordResetDialogSubmit = (form: RequestPasswordResetFormValues) => {
    const { userName, email, selectedHost } = form;
    const { host, port } = getHostPort(selectedHost);

    if (email) {
      webClient.request.authentication.resetPasswordChallenge({ userName, email, host, port });
    } else {
      setUserToResetPassword(userName);
      webClient.request.authentication.resetPasswordRequest({ userName, host, port });
    }
  };

  const handleResetPasswordDialogSubmit = ({
    userName,
    token,
    newPassword,
    selectedHost,
  }: ResetPasswordFormValues) => {
    const { host, port } = getHostPort(selectedHost);
    webClient.request.authentication.resetPassword({ userName, token, newPassword, host, port });
  };

  const skipTokenRequest = (userName: string) => {
    setUserToResetPassword(userName);

    setDialogState((s) => ({
      ...s,
      passwordResetRequestDialog: false,
      resetPasswordDialog: true,
    }));
  };

  return {
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
  };
}
