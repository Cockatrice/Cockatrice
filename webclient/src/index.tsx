import { ThemeProvider } from '@material-ui/styles';
import React from 'react';
import ReactDOM from 'react-dom';


import { AppShell } from 'containers';

import { materialTheme } from './material-theme';
import './i18n';

import './index.css';

const appWithMaterialTheme = () => (
  <ThemeProvider theme={materialTheme}>
    <AppShell />
  </ThemeProvider>
);

ReactDOM.render(appWithMaterialTheme(), document.getElementById('root'));
