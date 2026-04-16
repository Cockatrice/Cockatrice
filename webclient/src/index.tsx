// MUST be first: installs BigInt.prototype.toJSON before any module that
// creates the Redux store or connects to Redux DevTools.
import './polyfills';

import { StrictMode, useRef } from 'react';
import { createRoot } from 'react-dom/client';
import { StyledEngineProvider } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';

import { initWebClient } from '@app/api';
import { AppShell } from '@app/containers';
import { materialTheme } from './material-theme';

import './i18n';
import './index.css';

function useInitWebClient() {
  const initialized = useRef(false);

  if (!initialized.current) {
    initialized.current = true;
    initWebClient();
  }
}

const AppWithMaterialTheme = () => {
  // Instantiate the WebClient singleton before any container renders or any
  // hook touches WebClient.instance.
  useInitWebClient();

  return (
    <StrictMode>
      <StyledEngineProvider injectFirst>
        <ThemeProvider theme={materialTheme}>
          <AppShell />
        </ThemeProvider>
      </StyledEngineProvider>
    </StrictMode>
  );
}

const container = document.getElementById('root');
const root = createRoot(container!);

root.render(<AppWithMaterialTheme />);
