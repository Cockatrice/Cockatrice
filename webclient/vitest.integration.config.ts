import react from '@vitejs/plugin-react';
import { defineConfig } from 'vitest/config';

// Integration tests exercise the full inbound/outbound webclient pipeline
// (ProtobufService → event handlers → persistence → Redux) with only the
// browser WebSocket constructor mocked. They live in `integration/` and run
// under their own config so they can use `isolate: true` without slowing down
// the unit suite (which relies on `isolate: false` for shared vi.mock state).
export default defineConfig({
  plugins: [react()],
  resolve: {
    tsconfigPaths: true,
  },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: ['./integration/src/helpers/setup.ts'],
    include: ['integration/src/**/*.spec.{ts,tsx}'],
    // App-suite tests render the full Login container against real Dexie
    // (fake-indexeddb) + real WebClient. Under CI/disk load the default
    // 5s timeout is tight; 10s leaves headroom without masking real hangs.
    testTimeout: 10000,
    coverage: {
      provider: 'v8',
      reporter: ['text', 'html'],
      reportsDirectory: './coverage/integration',
      include: [
        'src/websocket/**/*.{ts,tsx}',
        'src/store/**/*.{ts,tsx}',
        'src/api/**/*.{ts,tsx}',
      ],
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
