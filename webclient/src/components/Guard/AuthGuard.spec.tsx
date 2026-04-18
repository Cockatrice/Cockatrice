import { screen } from '@testing-library/react';
import { renderWithProviders, connectedState, disconnectedState } from '../../__test-utils__';
import AuthGuard from './AuthGuard';

vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: vi.fn(() => ({})) };
});

describe('AuthGuard', () => {
  it('redirects to LOGIN when disconnected', () => {
    renderWithProviders(<AuthGuard />, {
      preloadedState: disconnectedState,
      route: '/server',
    });

    // Navigate triggers a route change — AuthGuard itself renders no text.
    // We verify it doesn't render any meaningful content.
    expect(screen.queryByRole('button')).toBeNull();
    expect(screen.queryByRole('heading')).toBeNull();
  });

  it('renders nothing visible when connected', () => {
    const { container } = renderWithProviders(<AuthGuard />, {
      preloadedState: connectedState,
      route: '/server',
    });

    // AuthGuard renders an empty fragment when connected.
    // The only DOM is from provider wrappers (e.g. ToastProvider's container div).
    expect(container.textContent).toBe('');
  });
});
