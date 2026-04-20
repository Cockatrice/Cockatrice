// @critical Must be the first import. See .github/instructions/webclient.instructions.md#initialization-order.
import './polyfills';

import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { StyledEngineProvider } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';

import { WebClientProvider } from '@app/hooks';
import { AppShell } from '@app/containers';
import { materialTheme } from './material-theme';

import './i18n';
import './colors.css';
import './index.css';

const AppWithMaterialTheme = () => {
  return (
    <WebClientProvider>
      <StrictMode>
        <StyledEngineProvider injectFirst>
          <ThemeProvider theme={materialTheme}>
            <AppShell />
          </ThemeProvider>
        </StyledEngineProvider>
      </StrictMode>
    </WebClientProvider>
  );
};

const container = document.getElementById('root');
const root = createRoot(container!);

root.render(<AppWithMaterialTheme />);
