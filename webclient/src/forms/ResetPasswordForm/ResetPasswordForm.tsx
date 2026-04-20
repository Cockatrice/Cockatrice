import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { InputField, KnownHosts } from '@app/components';

import { useResetPasswordForm } from './useResetPasswordForm';

import './ResetPasswordForm.css';

const ResetPasswordForm = ({ onSubmit, userName }) => {
  const { t } = useTranslation();
  const { errorMessage } = useResetPasswordForm();

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (!values.token) {
      errors.token = t('Common.validation.required');
    }

    if (!values.newPassword) {
      errors.newPassword = t('Common.validation.required');
    } else if (values.newPassword.length < 8) {
      errors.newPassword = t('Common.validation.minChars', { count: 8 });
    }

    if (!values.passwordAgain) {
      errors.passwordAgain = t('Common.validation.required');
    } else if (values.newPassword !== values.passwordAgain) {
      errors.passwordAgain = t('Common.validation.passwordsMustMatch');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  };

  const handleOnSubmit = ({ userName, token, ...values }) => {
    userName = userName?.trim();
    token = token?.trim();

    onSubmit({ userName, token, ...values });
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate} initialValues={{ userName }}>
      {({ handleSubmit }) => (
        <form className='ResetPasswordForm' onSubmit={handleSubmit}>
          <div className='ResetPasswordForm-items'>
            <div className='ResetPasswordForm-item'>
              <Field
                label={t('Common.label.username')}
                name='userName'
                component={InputField}
                autoComplete='username'
                disabled={!!userName}
              />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field label={t('Common.label.token')} name='token' component={InputField} />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field
                label={t('Common.label.password')}
                name='newPassword'
                type='password'
                component={InputField}
                autoComplete='new-password'
              />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field
                label={t('Common.label.passwordAgain')}
                name='passwordAgain'
                type='password'
                component={InputField}
                autoComplete='new-password'
              />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field name='selectedHost' component={KnownHosts} disabled />
            </div>

            {errorMessage && (
              <div className='ResetPasswordForm-item'>
                <Typography color="error">{t('ResetPasswordForm.error')}</Typography>
              </div>
            )}
          </div>
          <Button className='ResetPasswordForm-submit rounded tall' color='primary' variant='contained' type='submit'>
            {t('ResetPasswordForm.label.reset')}
          </Button>
        </form>
      )}
    </Form>
  );
};

export default ResetPasswordForm;
