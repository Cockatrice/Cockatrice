import { useEffect, useState } from 'react';
import { useSelector } from 'react-redux';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import setFieldTouched from 'final-form-set-field-touched';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { CountryDropdown, InputField, KnownHosts } from '@app/components';
import { useReduxEffect } from '@app/hooks';
import { ServerDispatch, ServerSelectors, ServerTypes } from '@app/store';

import './RegisterForm.css';
import { useToast } from '@app/components';

const RegisterForm = ({ onSubmit }: RegisterFormProps) => {
  const { t } = useTranslation();
  const [emailRequired, setEmailRequired] = useState(false);
  const [emailError, setEmailError] = useState(null);
  const [passwordError, setPasswordError] = useState(null);
  const [userNameError, setUserNameError] = useState(null);
  const error = useSelector(ServerSelectors.getRegistrationError);
  const { openToast } = useToast({ key: 'registration-success', children: t('RegisterForm.toast.registerSuccess') })

  const onHostChange = () => setEmailRequired(false);
  const onEmailChange = () => emailError && setEmailError(null);
  const onPasswordChange = () => passwordError && setPasswordError(null);
  const onUserNameChange = () => userNameError && setUserNameError(null);

  useReduxEffect(() => {
    setEmailRequired(true);
  }, ServerTypes.REGISTRATION_REQUIRES_EMAIL);

  useReduxEffect(() => {
    openToast()
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

  const handleOnSubmit = ({ userName, email, realName, ...values }) => {
    ServerDispatch.clearRegistrationErrors();

    userName = userName?.trim();
    email = email?.trim();
    realName = realName?.trim();

    onSubmit({ userName, email, realName, ...values });
  }

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    } else if (userNameError) {
      errors.userName = userNameError;
    }

    if (!values.password) {
      errors.password = t('Common.validation.required');
    } else if (values.password.length < 8) {
      errors.password = t('Common.validation.minChars', { count: 8 });
    } else if (passwordError) {
      errors.password = passwordError;
    }

    if (!values.passwordConfirm) {
      errors.passwordConfirm = t('Common.validation.required');
    } else if (values.password !== values.passwordConfirm) {
      errors.passwordConfirm = t('Common.validation.passwordsMustMatch');
    }

    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    if (emailRequired && !values.email) {
      errors.email = t('Common.validation.required');
    } else if (emailError) {
      errors.email = emailError;
    }

    return errors;
  }

  return (
    <Form onSubmit={handleOnSubmit} validate={validate} mutators={{ setFieldTouched }}>
      {({ handleSubmit, form }) => {

        useEffect(() => {
          if (emailRequired) {
            form.mutators.setFieldTouched('email', true);
          }
        }, [emailRequired]);

        return (
          <>
            <form className="RegisterForm" onSubmit={handleSubmit}>
              <div className="RegisterForm-column">
                <div className="RegisterForm-item">
                  <Field label={t('Common.label.username')} name="userName" component={InputField} autoComplete="username" />
                  <OnChange name="userName">{onUserNameChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field
                    label={t('Common.label.password')}
                    name="password"
                    type="password"
                    component={InputField}
                    autoComplete='new-password'
                  />
                  <OnChange name="password">{onPasswordChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field
                    label={t('Common.label.confirmPassword')}
                    name="passwordConfirm"
                    type="password"
                    component={InputField}
                    autoComplete='new-password'
                  />
                </div>
                <div className="RegisterForm-item">
                  <Field name="selectedHost" component={KnownHosts} />
                  <OnChange name="selectedHost">{onHostChange}</OnChange>
                </div>
              </div>
              <div className="RegisterForm-column" >
                <div className="RegisterForm-item">
                  <Field label={t('Common.label.realName')} name="realName" component={InputField} />
                </div>
                <div className="RegisterForm-item">
                  <Field label={t('Common.label.email')} name="email" type="email" component={InputField} />
                  <OnChange name="email">{onEmailChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field label={t('Common.label.country')} name="country" component={CountryDropdown} />
                </div>
                <Button className="RegisterForm-submit tall" color="primary" variant="contained" type="submit">
                  { t('RegisterForm.label.register') }
                </Button>
              </div>
            </form>

            { error && (
              <div className="RegisterForm-item">
                <Typography color="error">{error}</Typography>
              </div>
            )}
          </>
        );
      }}

    </Form >
  );
};

interface RegisterFormProps {
  onSubmit: any;
}

export default RegisterForm;
