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
import { FormKey } from 'types';

import "./CardImportForm.css";
import { Select, MenuItem } from "@material-ui/core";

const CardImportForm = (props) => {
  const { handleSubmit, dispatch } = props;
  const [activeStep, setActiveStep] = React.useState(0);
  const steps = ['Source selection', 'Sets imported', 'Import tokens', 'Finished'];

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const getStepContent = (stepIndex) => {
    switch (stepIndex) {
      case 0: return (
        <Form className="cardImportForm" onSubmit={handleSubmit}>
          <div className="cardImportForm-item">
            <Field label="Download URL" name="cardDownloadUrl" component={InputField} />
          </div>

          <Button color="primary" onClick={handleNext}>Import</Button>
        </Form>
      );

      case 1: return (
        <div>
          <div>sets imported</div>
          <Button onClick={handleBack}>Go Back</Button>
          <Button onClick={handleNext}>Save</Button>
        </div>
      );

      case 2: return (
        <Form className="cardImportForm" onSubmit={handleSubmit}>
          <div className="cardImportForm-item">
            <Field label="Download URL" name="tokenDownloadUrl" component={InputField} />
          </div>

          <Button onClick={handleBack}>Go Back</Button>
          <Button onClick={handleNext}>Import</Button>
        </Form>
      );

      case 3: return (
        <div>
          <div>Finished!</div>

          <Button onClick={handleBack}>Go Back</Button>
        </div>
      );
    }
  };

  return (
    <div>
      <Stepper activeStep={activeStep} alternativeLabel>
        {steps.map((label) => (
          <Step key={label}>
            <StepLabel>{label}</StepLabel>
          </Step>
        ))}
      </Stepper>

      <div>{ getStepContent(activeStep) }</div>
    </div>
  );
}

const propsMap = {
  form: FormKey.CARD_IMPORT
};

const mapStateToProps = () => ({
  initialValues: {
    cardDownloadUrl: 'https://www.mtgjson.com/api/v5/AllPrintings.json.xz',
    tokenDownloadUrl: 'https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml'
  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(CardImportForm));
