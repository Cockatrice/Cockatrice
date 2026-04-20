import { Form, Field } from 'react-final-form';

import Button from '@mui/material/Button';
import Stepper from '@mui/material/Stepper';
import Step from '@mui/material/Step';
import StepLabel from '@mui/material/StepLabel';
import CircularProgress from '@mui/material/CircularProgress';

import { InputField, VirtualList } from '@app/components';

import { useCardImportForm } from './useCardImportForm';

import './CardImportForm.css';

const CardImportForm = ({ onSubmit: onClose }) => {
  const {
    loading,
    activeStep,
    importedCards,
    importedSets,
    error,
    handleBack,
    handleCardDownload,
    handleCardSave,
    handleTokenDownload,
  } = useCardImportForm();

  const steps = ['Imports sets', 'Save sets', 'Import tokens', 'Finished'];

  const getStepContent = (stepIndex) => {
    switch (stepIndex) {
      case 0: return (
        <Form
          onSubmit={handleCardDownload}
          initialValues={{ cardDownloadUrl: 'https://www.mtgjson.com/api/v5/AllPrintings.json' }}
        >
          {({ handleSubmit }) => (
            <form className='cardImportForm' onSubmit={handleSubmit}>
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
            </form>
          )}
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
        <Form
          onSubmit={handleTokenDownload}
          initialValues={{ tokenDownloadUrl: 'https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml' }}
        >
          {({ handleSubmit }) => (
            <form className='cardImportForm' onSubmit={handleSubmit}>
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
            </form>
          )}
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
        {getStepContent(activeStep)}
      </div>

      {loading && (
        <div className='loading'>
          <CircularProgress size={60} />
        </div>
      )}
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
        items={items}
        size={15}
      />
    </div>
  );
};

export default CardImportForm;
