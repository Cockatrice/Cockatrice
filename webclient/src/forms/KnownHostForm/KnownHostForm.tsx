// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form'
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import AnchorLink from '@mui/material/Link';

import { InputField } from 'components';

import './KnownHostForm.css';

const KnownHostForm = ({ host, onRemove, onSubmit }) => {
  const [confirmDelete, setConfirmDelete] = useState(false);
  const { t } = useTranslation();

  const validate = values => {
    const errors: any = {};

    if (!values.name) {
      errors.name = t('Common.validation.required');
    }

    if (!values.host) {
      errors.host = t('Common.validation.required');
    }

    if (!values.port) {
      errors.port = t('Common.validation.required');
    }

    if (Object.keys(errors).length) {
      return errors;
    }
  };

  const handleOnSubmit = ({ name, host, ...values }) => {
    name = name?.trim();
    host = host?.trim();

    onSubmit({ name, host, ...values });
  }

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
                <Button color="inherit" onClick={() => !confirmDelete ? setConfirmDelete(true) : onRemove(host)}>
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

const mapStateToProps = () => ({

});

export default connect(mapStateToProps)(KnownHostForm);
