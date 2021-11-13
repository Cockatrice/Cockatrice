// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, Field } from 'react-final-form'

import Button from "@material-ui/core/Button";
import AnchorLink from '@material-ui/core/Link';

import { InputField } from "components";

import "./KnownHostForm.css";

function KnownHostForm(props) {
  const { host, onSubmit } = props;

  return (
    <Form
      initialValues={{
        id: host?.id,
        name: host?.name,
        host: host?.host,
        port: host?.port,
      }}
      onSubmit={onSubmit}
      validate={values => {
        const errors: any = {};

        if (!values.name) {
          errors.name = 'Required'
        }

        if (!values.host) {
          errors.host = 'Required'
        }

        if (!values.port) {
          errors.port = 'Required'
        }

        if (Object.keys(errors).length) {
          return errors;
        }
      }}
    >
      {({ handleSubmit }) => (
        <form className="KnownHostForm" onSubmit={handleSubmit}>
          <div className="KnownHostForm-item">
            <Field label="Host Name" name="name" component={InputField} />
          </div>
          <div className="KnownHostForm-item">
            <Field label="Host Address" name="host" component={InputField} />
          </div>
          <div className="KnownHostForm-item">
            <Field label="Port" name="port" type="number" component={InputField} />
          </div>

          <Button className="KnownHostForm-submit" color="primary" variant="contained" type="submit">
            {host ? 'Save Changes' : 'Add Host' }
          </Button>

          <div className="KnownHostForm-help">
            <AnchorLink href='https://github.com/Cockatrice/Cockatrice/wiki' target='_blank'>Need help adding a new host?</AnchorLink>
          </div>
        </form>
      ) }
    </Form>
  );
}

const mapStateToProps = () => ({

});

export default connect(mapStateToProps)(KnownHostForm);
