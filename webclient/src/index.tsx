import { ThemeProvider } from '@material-ui/styles';
import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';

import { materialTheme } from './material-theme';

import { AppShell } from 'containers';

const appWithMaterialTheme = () => (
  <ThemeProvider theme={materialTheme}>
    <AppShell />
  </ThemeProvider>
);

ReactDOM.render(appWithMaterialTheme(), document.getElementById('root'));
