// MUST be first: installs BigInt.prototype.toJSON before any module that
// creates the Redux store or connects to Redux DevTools.
import './polyfills';

import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { StyledEngineProvider } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';

import { AppShell } from '@app/containers';
import { materialTheme } from './material-theme';

import './i18n';
import './index.css';

const AppWithMaterialTheme = () => (
  <StrictMode>
    <StyledEngineProvider injectFirst>
      <ThemeProvider theme={materialTheme}>
        <AppShell />
      </ThemeProvider>
    </StyledEngineProvider>
  </StrictMode>
);

const container = document.getElementById('root');
const root = createRoot(container!);

root.render(<AppWithMaterialTheme />);
