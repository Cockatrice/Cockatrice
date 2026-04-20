import { useState } from 'react';
import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { InputField } from '@app/components';
import type { FormErrors } from '@app/forms';
import { useReduxEffect } from '@app/hooks';
import { ServerTypes } from '@app/store';

import './AccountActivationForm.css';

export interface AccountActivationFormValues {
  token: string;
}

interface AccountActivationFormProps {
  onSubmit: (values: AccountActivationFormValues) => void;
}

const AccountActivationForm = ({ onSubmit }: AccountActivationFormProps) => {
  const [errorMessage, setErrorMessage] = useState(false);
  const { t } = useTranslation();

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.ACCOUNT_ACTIVATION_FAILED, []);

  const handleOnSubmit = ({ token, ...values }: AccountActivationFormValues) => {
    setErrorMessage(false);

    onSubmit({ ...values, token: token?.trim() });
  };

  const validate = (values: Partial<AccountActivationFormValues>): FormErrors<AccountActivationFormValues> => {
    const errors: FormErrors<AccountActivationFormValues> = {};

    if (!values.token) {
      errors.token = t('Common.validation.required');
    }

    return errors;
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit }) => {
        return (
          <form className="AccountActivationForm" onSubmit={handleSubmit}>
            <div className="AccountActivationForm-item">
              <Field label={t('Common.label.token')} name="token" component={InputField} />
            </div>

            {errorMessage && (
              <div className="AccountActivationForm-error">
                <Typography color="error">{ t('AccountActivationForm.error.failed') }</Typography>
              </div>
            )}

            <Button className="AccountActivationForm-submit rounded tall" color="primary" variant="contained" type="submit">
              { t('AccountActivationForm.label.activate') }
            </Button>
          </form>
        );
      }}
    </Form>
  );
};

export default AccountActivationForm;
