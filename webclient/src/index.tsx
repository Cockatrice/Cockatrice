import { Theme, StyledEngineProvider } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';
import React from 'react';
import ReactDOM from 'react-dom';

import { AppShell } from 'containers';

import { materialTheme } from './material-theme';
import './i18n';

import './index.css';

declare module '@mui/styles/defaultTheme' {
  // eslint-disable-next-line @typescript-eslint/no-empty-interface
  interface DefaultTheme extends Theme {}
}

const AppWithMaterialTheme = () => (
  <StyledEngineProvider injectFirst>
    <ThemeProvider theme={materialTheme}>
      <AppShell />
    </ThemeProvider>
  </StyledEngineProvider>
);

ReactDOM.render(<AppWithMaterialTheme />, document.getElementById('root'));
