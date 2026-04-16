import js from '@eslint/js';
import tseslint from 'typescript-eslint';
import globals from 'globals';
import { boundariesConfig } from './eslint.boundaries.mjs';

export default tseslint.config(
  // Global ignores
  { ignores: ['node_modules/**', 'build/**', 'public/pb/**', 'src/generated/**'] },

  // Base JS recommended
  js.configs.recommended,

  // TypeScript recommended (sets up parser + plugin)
  ...tseslint.configs.recommended,

  // Enforce module boundaries
  ...boundariesConfig,

  // Project-specific config
  {
    languageOptions: {
      ecmaVersion: 2020,
      sourceType: 'module',
      parserOptions: {
        ecmaFeatures: { jsx: true },
      },
      globals: {
        ...globals.browser,
        ...globals.es2020,
      },
    },
    rules: {
      // TypeScript overrides
      '@typescript-eslint/no-unused-vars': ['warn', { varsIgnorePattern: '^_', argsIgnorePattern: '^_', caughtErrorsIgnorePattern: '^_' }],
      '@typescript-eslint/no-explicit-any': 'off',
      '@typescript-eslint/no-require-imports': 'off',
      '@typescript-eslint/no-empty-object-type': 'off',
      '@typescript-eslint/no-unsafe-function-type': 'off',

      // Disable new rules not in original config
      'prefer-const': 'off',
      'no-extra-boolean-cast': 'off',
      'no-case-declarations': 'off',
      'preserve-caught-error': 'off',

      // Spacing / formatting
      'array-bracket-spacing': ['error', 'never'],
      'arrow-spacing': ['error', { before: true, after: true }],
      'block-spacing': ['error', 'always'],
      'brace-style': ['error', '1tbs', { allowSingleLine: false }],
      'comma-spacing': ['error', { before: false, after: true }],
      'comma-style': ['error', 'last'],
      'computed-property-spacing': ['error', 'never'],
      'curly': ['error', 'all'],
      'dot-location': ['error', 'property'],
      'eol-last': ['error'],
      'func-names': ['warn'],
      'indent': ['error', 2, { SwitchCase: 1 }],
      'key-spacing': ['error', { beforeColon: false, afterColon: true }],
      'keyword-spacing': ['error'],
      'linebreak-style': ['error', process.platform === 'win32' ? 'windows' : 'unix'],
      'max-len': ['error', { code: 140 }],
      'no-eq-null': ['off'],
      'no-func-assign': ['error'],
      'no-inline-comments': ['error'],
      'no-mixed-spaces-and-tabs': ['error'],
      'no-multi-spaces': ['error'],
      'no-trailing-spaces': ['error'],
      'no-var': ['error'],
      'object-curly-spacing': ['error', 'always'],
      'one-var': ['error', 'never'],
      'one-var-declaration-per-line': ['error'],
      'quotes': ['error', 'single'],
      'semi-spacing': ['error', { before: false, after: true }],
      'space-before-blocks': ['error'],
      'space-before-function-paren': ['error', { asyncArrow: 'always', anonymous: 'never', named: 'never' }],
      'space-in-parens': ['error', 'never'],
      'space-infix-ops': ['error'],
      'space-unary-ops': ['error', { words: true, nonwords: false }],
    },
  },
);
