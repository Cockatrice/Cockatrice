import { connect } from 'react-redux';
import { useTranslation } from 'react-i18next';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';

import './Unsupported.css';

const Unsupported = () => {
  const { t } = useTranslation();

  return (
    <div className='Unsupported'>
      <Paper className='Unsupported-paper'>
        <div className='Unsupported-paper__header'>
          <Typography variant="h1">{ t('UnsupportedContainer.title') }</Typography>
          <Typography variant="subtitle1">{ t('UnsupportedContainer.subtitle1') }</Typography>
        </div>

        <Typography variant="subtitle2">{ t('UnsupportedContainer.subtitle2') }</Typography>
      </Paper>
    </div>
  );
};

const mapStateToProps = state => ({

});

export default connect(mapStateToProps)(Unsupported);
