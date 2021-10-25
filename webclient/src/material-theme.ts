import { createMuiTheme } from '@material-ui/core';

export const materialTheme = createMuiTheme({
  overrides: {
  //   MuiCssBaseline: {
  //     '@global': {
  //       '@font-face': [],
  //     },
  //   },
    MuiButton: {
      root: {
        fontWeight: 'bold',
        textTransform: 'none',

        '&.rounded': {
          // 'border-radius': '50px',
        },

        '&.tall': {
          'height': '40px',
        },
      },
    },
    MuiOutlinedInput: {
      root: {
        '&.Mui-focused .MuiOutlinedInput-notchedOutline': {
          borderWidth: '1px',
        },

        '.rounded &': {
          // 'border-radius': '50px',
        },

        '.tall &': {
          'height': '40px',
        },
      },
    },
  },

  palette: {
    background: {
      default: 'dimgrey',
      paper: '#FFFFFF',
    },
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
    success: {
      main: '#6CDF39',
      light: '#6CDF39',
      // dark: '',
      // contrastText: '',
    },
  },

  typography: {
    fontSize: 12,

    h1: {
      fontSize: 28,
      fontWeight: 'bold',
    },
    // h2: {},
    // h3: {},
    // h4: {},
    // h5: {},
    // h6: {},
    subtitle1: {
      fontSize: 14,
      fontWeight: 'bold',
      lineHeight: 1.4,
      color: '#9E9E9E',
    },
    subtitle2: {
      lineHeight: 1.4,
      color: '#9E9E9E',
    },
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
