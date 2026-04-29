import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Divider from '@mui/material/Divider';
import Paper from '@mui/material/Paper';

import { InputField, CheckboxField } from '@app/components';

import './SearchForm.css';

export interface SearchFormValues {
  userName?: string;
  ipAddress?: string;
  gameName?: string;
  gameId?: string;
  message?: string;
  logLocation?: {
    room?: boolean;
    game?: boolean;
    chat?: boolean;
  };
}

interface SearchFormProps {
  onSubmit: (values: SearchFormValues) => void;
}

const SearchForm = ({ onSubmit }: SearchFormProps) => {
  const { t } = useTranslation();

  return (
    <Form onSubmit={onSubmit}>
      {({ handleSubmit }) => (
        <Paper className="log-search">
          <form className="log-search__form" onSubmit={handleSubmit}>
            <div className="log-search__form-item">
              <Field label={t('SearchForm.label.userName')} name="userName" component={InputField} />
            </div>
            <div className="log-search__form-item">
              <Field label={t('SearchForm.label.ipAddress')} name="ipAddress" component={InputField} />
            </div>
            <div className="log-search__form-item">
              <Field label={t('SearchForm.label.gameName')} name="gameName" component={InputField} />
            </div>
            <div className="log-search__form-item">
              <Field label={t('SearchForm.label.gameId')} name="gameId" component={InputField} />
            </div>
            <div className="log-search__form-item">
              <Field label={t('SearchForm.label.message')} name="message" component={InputField} />
            </div>
            <Divider />
            <div className="log-search__form-item log-location">
              <Field label={t('SearchForm.label.rooms')} name="logLocation.room" component={CheckboxField} />
              <Field label={t('SearchForm.label.games')} name="logLocation.game" component={CheckboxField} />
              <Field label={t('SearchForm.label.chats')} name="logLocation.chat" component={CheckboxField} />
            </div>
            <Divider />
            <Button className="log-search__form-submit" color="primary" variant="contained" type="submit">
              {t('SearchForm.button.search')}
            </Button>
          </form>
        </Paper>
      )}
    </Form>
  );
};

export default SearchForm;
