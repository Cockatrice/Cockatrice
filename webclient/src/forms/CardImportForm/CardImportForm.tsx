// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from "react-redux";
import { Form, Field, reduxForm} from "redux-form"

import Button from "@material-ui/core/Button";
import Stepper from '@material-ui/core/Stepper';
import Step from '@material-ui/core/Step';
import StepLabel from '@material-ui/core/StepLabel';

import { InputField, ScrollToBottomOnChanges, VirtualList } from "components";
import { cardImporterService, CardDTO, SetDTO, TokenDTO } from 'services';
import { FormKey } from 'types';

import "./CardImportForm.css";

const CardImportForm = (props) => {
  const { handleSubmit, onSubmit:onClose } = props;

  const [activeStep, setActiveStep] = React.useState(0);
  const [importedCards, setImportedCards] = React.useState([]);
  const [importedSets, setImportedSets] = React.useState([]);

  const steps = ['Source selection', 'Sets imported', 'Import tokens', 'Finished'];

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const handleCardDownload = ({ cardDownloadUrl }) => {
    cardImporterService.importCards(cardDownloadUrl)
      .then(({ cards, sets }) => {
        setImportedCards(cards);
        setImportedSets(sets);

        handleNext();
      })
      .catch(e => {
        // TODO: render error in UI
        console.log(e);
      });
  }

  const handleCardSave = async () => {
    await CardDTO.bulkAdd(importedCards);
    await SetDTO.bulkAdd(importedSets);

    handleNext();
  }

  const handleTokenDownload = ({ tokenDownloadUrl }) => {
    cardImporterService.importTokens(tokenDownloadUrl)
      .then(async tokens => {
        await TokenDTO.bulkAdd(tokens);
        handleNext();
      })
      .catch(e => {
        // TODO: render error in UI
        console.log(e);
      });
  }

  const BackButton = () => (
    <Button onClick={handleBack}>Go Back</Button>
  );

  const getStepContent = (stepIndex) => {
    switch (stepIndex) {
      case 0: return (
        <Form className="cardImportForm" onSubmit={handleSubmit(handleCardDownload)}>
          <div className="cardImportForm-item">
            <Field label="Download URL" name="cardDownloadUrl" component={InputField} />
          </div>

          <Button color="primary" type="submit">Import</Button>
        </Form>
      );

      case 1: return (
        <div>
          <div>
            <CardsImported cards={importedCards} sets={importedSets} />
          </div>
          <BackButton />
          <Button color="primary" onClick={handleCardSave}>Save</Button>
        </div>
      );

      case 2: return (
        <Form className="cardImportForm" onSubmit={handleSubmit(handleTokenDownload)}>
          <div className="cardImportForm-item">
            <Field label="Download URL" name="tokenDownloadUrl" component={InputField} />
          </div>

          <BackButton />
          <Button color="primary" type="submit">Import</Button>
        </Form>
      );

      case 3: return (
        <div>
          <div>Finished!</div>

          <BackButton />
          <Button color="primary" onClick={onClose}>Done</Button>
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

const CardsImported = ({ cards, sets }) => {
  const items = [
    ...sets.map(set => (
      <div>{set.name}: {set.cards.length} cards imported</div>
    ) ),

    (
      <div>
        <strong>Import finished: {cards.length} cards.</strong>
      </div>
    )
  ];

  return (
    <div className="card-import-list">
      <ScrollToBottomOnChanges changes={sets} content={(
        <VirtualList
          itemKey={(index) => index }
          items={items}
          size={15}
        />
      )} />
    </div>
  );
};

const propsMap = {
  form: FormKey.CARD_IMPORT,
  onClose: Function
};

const mapStateToProps = () => ({
  initialValues: {
    cardDownloadUrl: 'https://www.mtgjson.com/api/v5/AllPrintings.json',
    tokenDownloadUrl: 'https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml'
  },
});

export default connect(mapStateToProps)(reduxForm(propsMap)(CardImportForm));
