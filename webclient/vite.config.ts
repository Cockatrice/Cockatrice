import { fileURLToPath } from 'node:url';
import path from 'node:path';
import react from '@vitejs/plugin-react';
import { defineConfig } from 'vitest/config';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const srcPath = (...segments: string[]) => path.resolve(__dirname, 'src', ...segments);

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      '@app/api': srcPath('api/index.ts'),
      '@app/components': srcPath('components/index.ts'),
      '@app/containers': srcPath('containers/index.ts'),
      '@app/dialogs': srcPath('dialogs/index.ts'),
      '@app/forms': srcPath('forms/index.ts'),
      '@app/hooks': srcPath('hooks/index.ts'),
      '@app/images': srcPath('images/index.ts'),
      '@app/services': srcPath('services/index.ts'),
      '@app/store': srcPath('store/index.ts'),
      '@app/types': srcPath('types/index.ts'),
      '@app/utils': srcPath('utils/index.ts'),
      '@app/websocket/types': srcPath('websocket/types/index.ts'),
      '@app/websocket': srcPath('websocket/index.ts'),
      '@app/generated': srcPath('generated/index.ts'),
    },
  },
  optimizeDeps: {
    include: [
      '@mui/material',
      '@mui/material/styles',
      '@mui/icons-material',
      '@emotion/react',
      '@emotion/styled',
      '@dnd-kit/core',
      '@dnd-kit/utilities',
      '@reduxjs/toolkit',
      'react-redux',
      'react',
      'react-dom',
      'react-dom/client',
      'react/jsx-runtime',
      'react-router-dom',
      'i18next',
      'react-i18next',
      '@testing-library/react',
      '@testing-library/jest-dom/vitest',
      '@bufbuild/protobuf',
    ],
  },
  publicDir: 'public',
  build: {
    outDir: 'build',
  },
  server: {
    open: true,
    watch: {
      ignored: ['build', 'coverage', 'integration']
    }
  },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: ['./src/setupTests.ts'],
    include: ['src/**/*.spec.{ts,tsx}'],
    exclude: ['node_modules', 'build', 'integration', 'coverage'],
    isolate: true,
    pool: 'threads',
    maxWorkers: 4,
    testTimeout: 10000,
    coverage: {
      provider: 'v8',
      reporter: ['text', 'html'],
      reportsDirectory: './coverage/testing',
      include: ['src/**/*.{ts,tsx}'],
      exclude: [
        'src/generated/**',
        'src/**/*.spec.{ts,tsx}',
        'src/**/__mocks__/**',
        'src/setupTests.ts',
        'src/polyfills.ts',
      ],
    },
  },
});
