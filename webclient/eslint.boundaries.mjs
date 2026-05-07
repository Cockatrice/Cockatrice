import boundaries from 'eslint-plugin-boundaries';

const elements = [
  { type: 'api', pattern: ['src/api/**'] },
  { type: 'components', pattern: ['src/components/**'] },
  { type: 'containers', pattern: ['src/containers/**'] },
  { type: 'dialogs', pattern: ['src/dialogs/**'] },
  { type: 'forms', pattern: ['src/forms/**'] },
  { type: 'hooks', pattern: ['src/hooks/**'] },
  { type: 'images', pattern: ['src/images/**'] },
  { type: 'services', pattern: ['src/services/**'] },
  { type: 'store', pattern: ['src/store/**'] },
  { type: 'types', pattern: ['src/types/**'] },
  { type: 'utils', pattern: ['src/utils/**'] },
];

const types = (...types) => types.map((type) => ({ to: { type } }));

const rules = [
  { from: { type: 'types' }, allow: [] },
  { from: { type: 'utils' }, allow: types('types') },

  { from: { type: 'store' }, allow: types('types', 'utils') },
  { from: { type: 'api' }, allow: types('store', 'types', 'utils') },

  { from: { type: 'images' }, allow: types('types') },
  { from: { type: 'services' }, allow: types('api', 'store', 'types', 'utils') },
  { from: { type: 'hooks' }, allow: types('api', 'services', 'store', 'types', 'utils') },

  {
    from: { type: 'components' },
    allow: types('api', 'dialogs', 'forms', 'hooks', 'images', 'services', 'store', 'types', 'utils')
  },
  {
    from: { type: 'containers' },
    allow: types('api', 'components', 'dialogs', 'forms', 'hooks', 'images', 'services', 'store', 'types', 'utils')
  },
  { from: { type: 'dialogs' }, allow: types('components', 'forms', 'hooks', 'services', 'store', 'types', 'utils') },
  { from: { type: 'forms' }, allow: types('components', 'hooks', 'services', 'store', 'types', 'utils') },
];

export const boundariesConfig = [
  {
    plugins: { boundaries },
    settings: {
      'boundaries/elements': elements,
      'import/resolver': {
        typescript: {
          alwaysTryTypes: true,
          project: './tsconfig.json',
        },
      },
    },
    rules: {
      'boundaries/dependencies': ['error', {
        default: 'disallow',
        rules,
      }],
    },
  },
  {
    files: ['**/*.spec.*'],
    rules: { 'boundaries/dependencies': 'off' },
  },
];
