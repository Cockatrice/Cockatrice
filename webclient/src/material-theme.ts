import { createTheme } from '@material-ui/core/styles';

const palette = {
  background: {
    default: 'rgb(35, 35, 35)',
    paper: '#FFFFFF',
  },
  primary: {
    main: '#7033DB',
    light: 'rgba(112, 51, 219, .3)',
    dark: '#401C7F',
    contrastText: '#FFFFFF',
  },
  grey: {
    50: '#fafafa',
    100: '#f5f5f5',
    200: '#eeeeee',
    300: '#e0e0e0',
    400: '#bdbdbd',
    500: '#9e9e9e',
    600: '#757575',
    700: '#616161',
    800: '#424242',
    900: '#212121',
    A100: '#d5d5d5',
    A200: '#aaaaaa',
    A400: '#303030',
    A700: '#616161',
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
};

export const materialTheme = createTheme({
  palette,

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

    MuiCheckbox: {
      root: {
        '& .MuiSvgIcon-root': {
          width: '.75em',
          height: '.75em',
        },
      },
    },

    MuiFormControlLabel: {
      label: {
        fontSize: 12,
        fontWeight: 'bold',
        color: palette.primary.main,

      },
    },

    MuiInputLabel: {
      outlined: {
        transform: 'translate(1em, 1em) scale(1)',
      },
    },

    MuiLink: {
      root: {
        fontWeight: 'bold',
      },
    },

    MuiList: {
      root: {
        padding: '8px',

        '&.MuiList-padding': {
          paddingBottom: '4px',
        },

        '& .MuiButton-root': {
          width: '100%',
        },

        '& > .MuiButtonBase-root': {
          padding: '8px 16px',
          marginBottom: '4px',
          borderRadius: 0,
          justifyContent: 'space-between',
        },

        '& .MuiButtonBase-root.Mui-selected': {
          background: 'none',
          fontWeight: 'bold',
        },

        ['& .MuiButtonBase-root:hover, & .MuiButtonBase-root.Mui-selected:hover']: {
          background: palette.primary.light
        },
      },
    },

    MuiListItem: {
      root: {
      },
    },

    MuiInputBase: {
      formControl: {
        '& .MuiSelect-root svg': {
          display: 'none',
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
          height: '40px',
        },
      },
    },
  },

  typography: {
    fontSize: 12,
    fontFamily: 'Open Sans, sans-serif',

    h1: {
      fontSize: 28,
      fontWeight: 'bold',
    },
    h2: {
      fontSize: 24,
      fontWeight: 'bold',
    },
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
