import { useCallback } from 'react';
import { Form, Field, FormApi } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { InputField, KnownHosts } from '@app/components';
import type { FormErrors } from '@app/forms';
import { HostDTO } from '@app/services';

import { useRequestPasswordResetForm } from './useRequestPasswordResetForm';

import './RequestPasswordResetForm.css';

export interface RequestPasswordResetFormValues {
  userName: string;
  email?: string;
  selectedHost: HostDTO;
}

interface RequestPasswordResetFormProps {
  onSubmit: (values: RequestPasswordResetFormValues) => void;
  skipTokenRequest: (userName: string) => void;
}

interface HostChangePayload {
  userName?: string;
}

const RequestPasswordResetForm = ({ onSubmit, skipTokenRequest }: RequestPasswordResetFormProps) => {
  const { t } = useTranslation();
  const { errorMessage, setErrorMessage, isMFA, setIsMFA } = useRequestPasswordResetForm();

  const handleOnSubmit = ({ userName, email, ...values }: RequestPasswordResetFormValues) => {
    setErrorMessage(false);

    onSubmit({
      ...values,
      userName: userName?.trim(),
      email: email?.trim(),
    });
  };

  const validate = (values: Partial<RequestPasswordResetFormValues>): FormErrors<RequestPasswordResetFormValues> => {
    const errors: FormErrors<RequestPasswordResetFormValues> = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (isMFA && !values.email) {
      errors.email = t('Common.validation.required');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => (
        <RequestPasswordResetFormBody
          handleSubmit={handleSubmit}
          form={form}
          errorMessage={errorMessage}
          isMFA={isMFA}
          setIsMFA={setIsMFA}
          skipTokenRequest={skipTokenRequest}
        />
      )}
    </Form>
  );
};

interface BodyProps {
  handleSubmit: (event?: React.SyntheticEvent) => void;
  form: FormApi;
  errorMessage: boolean;
  isMFA: boolean;
  setIsMFA: (v: boolean) => void;
  skipTokenRequest: (userName: string) => void;
}

const RequestPasswordResetFormBody = ({
  handleSubmit,
  form,
  errorMessage,
  isMFA,
  setIsMFA,
  skipTokenRequest,
}: BodyProps) => {
  const { t } = useTranslation();

  const onHostChange = useCallback(({ userName }: HostChangePayload) => {
    form.change('userName', userName);
    setIsMFA(false);
  }, [form, setIsMFA]);

  return (
    <form className="RequestPasswordResetForm" onSubmit={handleSubmit}>
      <div className="RequestPasswordResetForm-items">
        <div className="RequestPasswordResetForm-item">
          <Field label={t('Common.label.username')} name="userName" component={InputField} autoComplete="username" disabled={isMFA} />
        </div>
        {isMFA ? (
          <div className="RequestPasswordResetForm-item">
            <Field label={t('Common.label.email')} name="email" type="email" component={InputField} autoComplete="email" />
            <div>{t('RequestPasswordResetForm.mfaEnabled')}</div>
          </div>
        ) : null}
        <div className="RequestPasswordResetForm-item selectedHost">
          <Field name='selectedHost' component={KnownHosts} disabled={isMFA} />
          <OnChange name="selectedHost">{onHostChange}</OnChange>
        </div>

        {errorMessage && (
          <div className="RequestPasswordResetForm-item">
            <Typography color="error">{t('RequestPasswordResetForm.error')}</Typography>
          </div>
        )}
      </div>

      <Button className="RequestPasswordResetForm-submit rounded tall" color="primary" variant="contained" type="submit">
        {t('RequestPasswordResetForm.request')}
      </Button>

      <div>
        <Button color="primary" onClick={() => skipTokenRequest(form.getState().values.userName)}>
          {t('RequestPasswordResetForm.skipRequest')}
        </Button>
      </div>
    </form>
  );
};

export default RequestPasswordResetForm;
