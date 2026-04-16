export { initWebClient } from './initWebClient';
export { createWebClientResponse } from './response';
export { createWebClientRequest } from './request';

import { createWebClientRequest } from './request';

/**
 * UI-facing request surface. The request implementations are created once
 * at module load. They access `WebClient.instance` at call time (via lazy
 * internal references), so the singleton only needs to exist by the first
 * actual command send.
 */
export const request = createWebClientRequest();
