import { screen } from '@testing-library/react';
import { renderWithProviders, connectedState, makeUser, createMockWebClient } from '../../__test-utils__';
import UserDisplay from './UserDisplay';

const mockWebClient = createMockWebClient();

vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: vi.fn(() => mockWebClient) };
});

vi.mock('@app/images', () => ({
  Images: { Countries: { us: 'us.png', de: 'de.png' } },
}));

describe('UserDisplay', () => {
  it('renders user name', () => {
    const user = makeUser({ name: 'TestPlayer', country: 'us' });
    renderWithProviders(<UserDisplay user={user} />, {
      preloadedState: connectedState,
    });

    expect(screen.getByText('TestPlayer')).toBeInTheDocument();
  });

  it('renders country flag image', () => {
    const user = makeUser({ name: 'TestPlayer', country: 'us' });
    renderWithProviders(<UserDisplay user={user} />, {
      preloadedState: connectedState,
    });

    const img = screen.getByAltText('us');
    expect(img).toBeInTheDocument();
    expect(img).toHaveAttribute('src', 'us.png');
  });

  it('renders link to player profile', () => {
    const user = makeUser({ name: 'TestPlayer', country: 'us' });
    renderWithProviders(<UserDisplay user={user} />, {
      preloadedState: connectedState,
    });

    const link = screen.getByRole('link', { name: /TestPlayer/ });
    expect(link).toHaveAttribute('href', '/player/TestPlayer');
  });
});
