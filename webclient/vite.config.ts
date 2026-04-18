import react from '@vitejs/plugin-react';
import { defineConfig } from 'vite';

export default defineConfig({
  plugins: [react()],
  resolve: {
    tsconfigPaths: true,
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
    isolate: true,
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
