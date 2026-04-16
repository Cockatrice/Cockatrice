import boundaries from 'eslint-plugin-boundaries';

const elements = [
  { type: 'api', pattern: ['src/api/**'] },
  { type: 'components', pattern: ['src/components/**'] },
  { type: 'containers', pattern: ['src/containers/**'] },
  { type: 'dialogs', pattern: ['src/dialogs/**'] },
  { type: 'forms', pattern: ['src/forms/**'] },
  { type: 'generated', pattern: ['src/generated/**'] },
  { type: 'hooks', pattern: ['src/hooks/**'] },
  { type: 'images', pattern: ['src/images/**'] },
  { type: 'services', pattern: ['src/services/**'] },
  { type: 'store', pattern: ['src/store/**'] },
  { type: 'types', pattern: ['src/types/**'] },
  { type: 'websocket', pattern: ['src/websocket/**'] },
];

const types = (...types) => types.map((type) => ({ to: { type } }));

const rules = [
  { from: { type: 'generated' }, allow: [] },
  { from: { type: 'types' }, allow:  types('generated', 'websocket')  },

  { from: { type: 'websocket' }, allow: types('generated') },
  { from: { type: 'store' }, allow: types('types') },
  { from: { type: 'api' }, allow: types('types', 'store', 'websocket') },

  { from: { type: 'hooks' }, allow: types('services', 'types') },
  { from: { type: 'images' }, allow: types('types') },
  { from: { type: 'services' }, allow: types('api', 'store', 'types') },

  { from: { type: 'components' }, allow: types('api', 'dialogs', 'forms', 'hooks', 'images', 'services', 'types', 'store') },
  { from: { type: 'containers' }, allow: types('api', 'components', 'dialogs', 'forms', 'hooks', 'images', 'services', 'types', 'store') },
  { from: { type: 'dialogs' }, allow: types('components', 'forms', 'hooks', 'services', 'types', 'store') },
  { from: { type: 'forms' }, allow: types('components', 'hooks', 'types', 'services', 'store') },
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
