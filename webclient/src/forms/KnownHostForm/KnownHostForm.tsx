import { useState } from 'react';
import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import AnchorLink from '@mui/material/Link';

import { InputField } from '@app/components';
import type { FormErrors } from '@app/forms';
import type { HostDTO } from '@app/services';

import './KnownHostForm.css';

export interface KnownHostFormValues {
  id?: number;
  name: string;
  host: string;
  port: string;
}

interface KnownHostFormProps {
  host?: HostDTO;
  onRemove: (host: HostDTO) => void;
  onSubmit: (values: KnownHostFormValues) => void;
}

const KnownHostForm = ({ host, onRemove, onSubmit }: KnownHostFormProps) => {
  const [confirmDelete, setConfirmDelete] = useState(false);
  const { t } = useTranslation();

  const validate = (values: Partial<KnownHostFormValues>): FormErrors<KnownHostFormValues> => {
    const errors: FormErrors<KnownHostFormValues> = {};

    if (!values.name) {
      errors.name = t('Common.validation.required');
    }

    if (!values.host) {
      errors.host = t('Common.validation.required');
    }

    if (!values.port) {
      errors.port = t('Common.validation.required');
    }

    return errors;
  };

  const handleOnSubmit = ({ name, host: hostValue, ...values }: KnownHostFormValues) => {
    onSubmit({
      ...values,
      name: name?.trim(),
      host: hostValue?.trim(),
    });
  };

  const handleRemoveClick = () => {
    if (!host) {
      return;
    }
    if (!confirmDelete) {
      setConfirmDelete(true);
      return;
    }
    onRemove(host);
  };

  return (
    <Form
      initialValues={{
        id: host?.id,
        name: host?.name,
        host: host?.host,
        port: host?.port,
      }}
      onSubmit={handleOnSubmit}
      validate={validate}
    >
      {({ handleSubmit }) => (
        <form className="KnownHostForm" onSubmit={handleSubmit}>
          <div className="KnownHostForm-item">
            <Field label={t('Common.label.hostName')} name="name" component={InputField} />
          </div>
          <div className="KnownHostForm-item">
            <Field label={t('Common.label.hostAddress')} name="host" component={InputField} />
          </div>
          <div className="KnownHostForm-item">
            <Field label={t('Common.label.port')} name="port" type="number" component={InputField} />
          </div>

          <Button className="KnownHostForm-submit" color="primary" variant="contained" type="submit">
            {host ? t('Common.label.saveChanges') : t('KnownHostForm.label.add') }
          </Button>

          <div className="KnownHostForm-actions">
            <div className="KnownHostForm-actions__delete">
              { host && (
                <Button color="inherit" onClick={handleRemoveClick}>
                  { !confirmDelete ? t('Common.label.delete') : t('Common.label.confirmSure') }
                </Button>
              ) }
            </div>
            <AnchorLink href='https://github.com/Cockatrice/Cockatrice/wiki/Public-Servers' target='_blank'>
              { t('KnownHostForm.label.find') }
            </AnchorLink>
          </div>
        </form>
      ) }
    </Form>
  );
};

export default KnownHostForm;
