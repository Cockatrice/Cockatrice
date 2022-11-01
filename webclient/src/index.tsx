import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { Theme, StyledEngineProvider } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';

import { AppShell } from './containers';
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
