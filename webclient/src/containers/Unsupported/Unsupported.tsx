import { connect } from 'react-redux';
import { withRouter } from 'react-router-dom';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';

import './Unsupported.css';

const Unsupported = () => {
  return (
    <div className='Unsupported'>
      <Paper className='Unsupported-paper'>
        <div className='Unsupported-paper__header'>
          <Typography variant="h1">Unsupported Browser</Typography>
          <Typography variant="subtitle1">Please update your browser and/or check your permissions.</Typography>
        </div>

        <Typography variant="subtitle2">Note: Private browsing causes some browsers to disable certain permissions or features.</Typography>
      </Paper>
    </div>
  );
};

const mapStateToProps = state => ({

});

export default withRouter(connect(mapStateToProps)(Unsupported));
