import { createMuiTheme } from '@material-ui/core';

export const materialTheme = createMuiTheme({
  // overrides: {
  //   MuiCssBaseline: {
  //     '@global': {
  //       '@font-face': [],
  //     },
  //   },
  //   MuiButton: {
  //     text: {
  //       color: 'white',
  //     },
  //   },
  // },

  palette: {
    primary: {
      main: '#7033DB',
      light: 'rgba(112, 51, 219, .3)',
      dark: '#401C7F',
      contrastText: '#FFFFFF',
    },
    // secondary: {
    //   main: '',
    //   light: '',
    //   dark: '',
    //   contrastText: '',
    // },
    // error: {
    //   main: '',
    //   light: '',
    //   dark: '',
    //   contrastText: '',
    // },
    // warning: {
    //   main: '',
    //   light: '',
    //   dark: '',
    //   contrastText: '',
    // },
    // info: {
    //   main: '',
    //   light: '',
    //   dark: '',
    //   contrastText: '',
    // },
    // success: {
    //   main: '',
    //   light: '',
    //   dark: '',
    //   contrastText: '',
    // },
  },

  typography: {
    fontSize: 12,

    // h1: {},
    // h2: {},
    // h3: {},
    // h4: {},
    // h5: {},
    // h6: {},
    // subtitle1: {},
    // subtitle2: {},
    // body1: {},
    // body2: {},
    // button: {},
    // caption: {},
    // overline: {},
  },

  spacing: 8,

  breakpoints: {
    values: {
      xs: 0,
      sm: 640,
      md: 768,
      lg: 1024,
      xl: 1280,
    },
  },
});
