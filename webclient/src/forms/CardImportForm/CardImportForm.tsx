// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from "react-redux";
import { Form, Field, reduxForm, change} from "redux-form"

import Button from "@material-ui/core/Button";
import Stepper from '@material-ui/core/Stepper';
import Step from '@material-ui/core/Step';
import StepLabel from '@material-ui/core/StepLabel';

import { InputField } from "components";
import { cardImporterService } from 'services';
import { FormKey, KnownHost, KnownHosts } from 'types';

import "./CardImportForm.css";
import { Select, MenuItem } from "@material-ui/core";

const CardImportForm = (props) => {
  const { handleSubmit, dispatch } = props;

  return (
    <Form className="cardImportForm" onSubmit={handleSubmit}>
      <div className="cardImportForm-item">
        <Field label="Download URL" name="download" component={InputField} />
      </div>
    </Form>
  );
}

const propsMap = {
  form: FormKey.CARD_IMPORT
};

const mapStateToProps = () => ({
  initialValues: {

  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(CardImportForm));
