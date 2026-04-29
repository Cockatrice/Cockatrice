import { ReactNode } from 'react';
import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Stepper from '@mui/material/Stepper';
import Step from '@mui/material/Step';
import StepLabel from '@mui/material/StepLabel';
import CircularProgress from '@mui/material/CircularProgress';

import { InputField, VirtualList } from '@app/components';
import type { App } from '@app/types';

import { useCardImportForm } from './useCardImportForm';

import './CardImportForm.css';

const CARDS_URL = 'https://www.mtgjson.com/api/v5/AllPrintings.json';
const TOKENS_URL = 'https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml';

interface CardImportFormProps {
  onSubmit: () => void;
}

interface BackButtonProps {
  click: () => void;
  disabled?: boolean;
}

const BackButton = ({ click, disabled }: BackButtonProps) => {
  const { t } = useTranslation();
  return (
    <Button onClick={click} disabled={disabled}>{t('CardImportForm.button.goBack')}</Button>
  );
};

interface ErrorMessageProps {
  error: string | null;
}

const ErrorMessage = ({ error }: ErrorMessageProps): ReactNode => (
  error ? <div className='error'>{error}</div> : null
);

interface CardsImportedProps {
  cards: App.Card[];
  sets: App.Set[];
}

const CardsImported = ({ cards, sets }: CardsImportedProps) => {
  const { t } = useTranslation();
  const items: ReactNode[] = [
    (
      <div key='import-summary'>
        <strong>{t('CardImportForm.message.importSummary', { count: cards.length })}</strong>
      </div>
    ),
    (<div key='spacer' className='spacer' />),
    ...sets.map(set => (
      <div key={set.code ?? set.name}>
        {t('CardImportForm.message.setSummary', { name: set.name, count: set.cards.length })}
      </div>
    )),
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

const CardImportForm = ({ onSubmit: onClose }: CardImportFormProps) => {
  const { t } = useTranslation();
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

  const steps = [
    t('CardImportForm.steps.importSets'),
    t('CardImportForm.steps.saveSets'),
    t('CardImportForm.steps.importTokens'),
    t('CardImportForm.steps.finished'),
  ];

  const getStepContent = (stepIndex: number): ReactNode => {
    switch (stepIndex) {
      case 0: return (
        <Form
          onSubmit={handleCardDownload}
          initialValues={{ cardDownloadUrl: CARDS_URL }}
        >
          {({ handleSubmit }) => (
            <form className='cardImportForm' onSubmit={handleSubmit}>
              <div className='cardImportForm-item'>
                <Field label={t('CardImportForm.label.downloadUrl')} name='cardDownloadUrl' component={InputField} />
              </div>

              <div className='cardImportForm-actions'>
                <Button color='primary' type='submit' disabled={loading}>
                  {t('CardImportForm.button.import')}
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
              {t('CardImportForm.button.save')}
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
          initialValues={{ tokenDownloadUrl: TOKENS_URL }}
        >
          {({ handleSubmit }) => (
            <form className='cardImportForm' onSubmit={handleSubmit}>
              <div className='cardImportForm-content'>
                <Field label={t('CardImportForm.label.downloadUrl')} name='tokenDownloadUrl' component={InputField} />
              </div>

              <div className='cardImportForm-actions'>
                <BackButton click={handleBack} disabled={loading} />
                <Button color='primary' type='submit' disabled={loading}>
                  {t('CardImportForm.button.import')}
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
          <div className='cardImportForm-content done'>{t('CardImportForm.message.finished')}</div>

          <div className='cardImportForm-actions'>
            <BackButton click={handleBack} disabled={loading} />
            <Button color='primary' onClick={onClose}>{t('CardImportForm.button.done')}</Button>
          </div>
        </div>
      );

      default:
        throw new Error(`CardImportForm: unknown step index ${stepIndex}`);
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

export default CardImportForm;
