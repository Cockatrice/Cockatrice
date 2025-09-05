import { Component, Suspense } from 'react';
import { Provider } from 'react-redux';
import { MemoryRouter as Router } from 'react-router-dom';
import CssBaseline from '@mui/material/CssBaseline';
import { store } from 'store';
import Routes from './AppShellRoutes';
import FeatureDetection from './FeatureDetection';

import './AppShell.css';

import { ToastProvider } from 'components/Toast';
import { RefreshGuardProvider } from 'components';

class AppShell extends Component {
  componentDidMount() {
    // RefreshGuardProvider now handles beforeunload events
    // Removed basic window.onbeforeunload handler
  }

  handleContextMenu(event) {
    event.preventDefault();
  }

  render() {
    return (
      <Suspense fallback="loading">
        <Provider store={store}>
          <CssBaseline />
          <ToastProvider>
            <div className="AppShell" onContextMenu={this.handleContextMenu}>
              <Router>
                <RefreshGuardProvider>
                  <FeatureDetection />
                  <Routes />
                </RefreshGuardProvider>
              </Router>
            </div>
          </ToastProvider>
        </Provider>
      </Suspense>
    );
  }
}

export default AppShell;
