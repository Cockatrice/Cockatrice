import { defineConfig } from 'vitest/config';
import viteConfig from './vite.config';

export default defineConfig({
  ...viteConfig,
  test: {
    ...viteConfig.test,
    setupFiles: ['./integration/src/helpers/setup.ts'],
    include: ['integration/src/**/*.spec.{ts,tsx}'],
    exclude: ['node_modules', 'build', 'coverage'],
    coverage: {
      ...viteConfig.test?.coverage,
      reportsDirectory: './coverage/integration',
      include: [
        'src/websocket/**/*.{ts,tsx}',
        'src/store/**/*.{ts,tsx}',
        'src/api/**/*.{ts,tsx}',
      ],
    },
  },
});
