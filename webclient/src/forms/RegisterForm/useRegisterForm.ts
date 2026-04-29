import { useState } from 'react';
import { useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';

import { useToast } from '@app/components';
import { useReduxEffect } from '@app/hooks';
import { ServerSelectors, ServerTypes } from '@app/store';

export interface RegisterForm {
  emailRequired: boolean;
  emailError: string | null;
  passwordError: string | null;
  userNameError: string | null;
  error: string | null;
  onHostChange: () => void;
  onEmailChange: () => void;
  onPasswordChange: () => void;
  onUserNameChange: () => void;
}

export function useRegisterForm(): RegisterForm {
  const { t } = useTranslation();
  const [emailRequired, setEmailRequired] = useState(false);
  const [emailError, setEmailError] = useState<string | null>(null);
  const [passwordError, setPasswordError] = useState<string | null>(null);
  const [userNameError, setUserNameError] = useState<string | null>(null);
  const error = useSelector(ServerSelectors.getRegistrationError);
  const { openToast } = useToast({
    key: 'registration-success',
    children: t('RegisterForm.toast.registerSuccess'),
  });

  const onHostChange = () => setEmailRequired(false);
  const onEmailChange = () => emailError && setEmailError(null);
  const onPasswordChange = () => passwordError && setPasswordError(null);
  const onUserNameChange = () => userNameError && setUserNameError(null);

  useReduxEffect(() => {
    setEmailRequired(true);
  }, ServerTypes.REGISTRATION_REQUIRES_EMAIL);

  useReduxEffect(() => {
    openToast();
  }, ServerTypes.REGISTRATION_SUCCESS);

  useReduxEffect(({ payload: { error } }) => {
    setEmailError(error);
  }, ServerTypes.REGISTRATION_EMAIL_ERROR);

  useReduxEffect(({ payload: { error } }) => {
    setPasswordError(error);
  }, ServerTypes.REGISTRATION_PASSWORD_ERROR);

  useReduxEffect(({ payload: { error } }) => {
    setUserNameError(error);
  }, ServerTypes.REGISTRATION_USERNAME_ERROR);

  return {
    emailRequired,
    emailError,
    passwordError,
    userNameError,
    error,
    onHostChange,
    onEmailChange,
    onPasswordChange,
    onUserNameChange,
  };
}
