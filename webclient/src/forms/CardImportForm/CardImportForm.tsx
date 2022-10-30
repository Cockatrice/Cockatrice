// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { connect } from 'react-redux';
import { Form, Field, reduxForm } from 'redux-form'

import Button from '@mui/material/Button';
import Stepper from '@mui/material/Stepper';
import Step from '@mui/material/Step';
import StepLabel from '@mui/material/StepLabel';
import CircularProgress from '@mui/material/CircularProgress';

import { InputField, VirtualList } from 'components';
import { cardImporterService, CardDTO, SetDTO, TokenDTO } from 'services';
import { FormKey } from 'types';

import './CardImportForm.css';

const CardImportForm = (props) => {
  const { handleSubmit, onSubmit: onClose } = props;

  const [loading, setLoading] = useState(false);
  const [activeStep, setActiveStep] = useState(0);
  const [importedCards, setImportedCards] = useState([]);
  const [importedSets, setImportedSets] = useState([]);
  const [error, setError] = useState(null);

  useEffect(() => {
    if (loading) {
      setError(null);
    }
  }, [loading])

  const steps = ['Imports sets', 'Save sets', 'Import tokens', 'Finished'];

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const handleCardDownload = ({ cardDownloadUrl }) => {
    setLoading(true);

    cardImporterService.importCards(cardDownloadUrl)
      .then(({ cards, sets }) => {
        setImportedCards(cards);
        setImportedSets(sets);

        handleNext();
      })
      .catch(({ message }) => setError(message))
      .finally(() => setLoading(false));
  };

  const handleCardSave = async () => {
    setLoading(true);

    try {
      await CardDTO.bulkAdd(importedCards);
      await SetDTO.bulkAdd(importedSets);

      handleNext();
    } catch (e) {
      console.error(e);
      setError('Failed to save cards');
    }

    setLoading(false);
  };

  const handleTokenDownload = ({ tokenDownloadUrl }) => {
    setLoading(true);

    cardImporterService.importTokens(tokenDownloadUrl)
      .then(async tokens => {
        await TokenDTO.bulkAdd(tokens);
        handleNext();
      })
      .catch(({ message }) => setError(message))
      .finally(() => setLoading(false));
  };

  const getStepContent = (stepIndex) => {
    switch (stepIndex) {
      case 0: return (
        <Form className='cardImportForm' onSubmit={handleSubmit(handleCardDownload)}>
          <div className='cardImportForm-item'>
            <Field label='Download URL' name='cardDownloadUrl' component={InputField} />
          </div>

          <div className='cardImportForm-actions'>
            <Button color='primary' type='submit' disabled={loading}>
              Import
            </Button>
          </div>

          <div className='cardImportForm-error'>
            <ErrorMessage error={error} />
          </div>
        </Form>
      );

      case 1: return (
        <div className='cardImportForm'>
          <div className='cardImportForm-content'>
            <CardsImported cards={importedCards} sets={importedSets} />
          </div>

          <div className='cardImportForm-actions'>
            <BackButton click={handleBack} disabled={loading} />
            <Button color='primary' onClick={handleCardSave} disabled={loading}>
              Save
            </Button>
          </div>

          <div className='cardImportForm-error'>
            <ErrorMessage error={error} />
          </div>
        </div>
      );

      case 2: return (
        <Form className='cardImportForm' onSubmit={handleSubmit(handleTokenDownload)}>
          <div className='cardImportForm-content'>
            <Field label='Download URL' name='tokenDownloadUrl' component={InputField} />
          </div>

          <div className='cardImportForm-actions'>
            <BackButton click={handleBack} disabled={loading} />
            <Button color='primary' type='submit' disabled={loading}>
              Import
            </Button>
          </div>

          <div className='cardImportForm-error'>
            <ErrorMessage error={error} />
          </div>
        </Form>
      );

      case 3: return (
        <div className='cardImportForm'>
          <div className='cardImportForm-content done'>Finished!</div>

          <div className='cardImportForm-actions'>
            <BackButton click={handleBack} disabled={loading} />
            <Button color='primary' onClick={onClose}>Done</Button>
          </div>
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

      <div>
        { getStepContent(activeStep) }
      </div>

      { loading && (
        <div className='loading'>
          <CircularProgress size={60} />
        </div>
      ) }
    </div>
  );
};

const BackButton = ({ click, disabled }) => (
  <Button onClick={click} disabled={disabled}>Go Back</Button>
);

const ErrorMessage = ({ error }) => {
  return error && (
    <div className='error'>{error}</div>
  );
};

const CardsImported = ({ cards, sets }) => {
  const items = [
    (
      <div>
        <strong>Import finished: {cards.length} cards.</strong>
      </div>
    ),

    (<div className='spacer' />),

    ...sets.map(set => (
      <div>{set.name}: {set.cards.length} cards imported</div>
    ))
  ];

  return (
    <div className='card-import-list'>
      <VirtualList
        itemKey={(index) => index }
        items={items}
        size={15}
      />
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
