import { Suspense, useEffect } from 'react';
import { Provider } from 'react-redux';
import { MemoryRouter as Router } from 'react-router-dom';
import CssBaseline from '@mui/material/CssBaseline';
import { store } from '@app/store';
import Routes from './AppShellRoutes';
import FeatureDetection from './FeatureDetection';

import './AppShell.css';

import { ToastProvider } from '@app/components';

function AppShell() {
  useEffect(() => {
    window.onbeforeunload = () => true;
    return () => {
      window.onbeforeunload = null;
    };
  }, []);

  return (
    <Suspense fallback="loading">
      <Provider store={store}>
        <CssBaseline />
        <ToastProvider>
          <div className="AppShell">
            <Router>
              <FeatureDetection />
              <Routes />
            </Router>
          </div>
        </ToastProvider>
      </Provider>
    </Suspense>
  );
}

export default AppShell;
