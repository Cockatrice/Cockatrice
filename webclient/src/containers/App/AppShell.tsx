import { Component, Suspense } from 'react';
import { Provider } from 'react-redux';
import { MemoryRouter as Router } from 'react-router-dom';
import CssBaseline from '@material-ui/core/CssBaseline';
import { store } from 'store';
import { Header } from 'components';
import Routes from './AppShellRoutes';
import FeatureDetection from './FeatureDetection';

import './AppShell.css';

import { ToastProvider } from 'components/Toast'

class AppShell extends Component {
  componentDidMount() {
    // @TODO (1)
    window.onbeforeunload = () => true;
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
                <Header />

                <FeatureDetection />
                <Routes />
              </Router>
            </div>
          </ToastProvider>
        </Provider>
      </Suspense>
    );
  }
}

export default AppShell;
