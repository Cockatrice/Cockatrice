import { screen } from '@testing-library/react';
import { renderWithProviders, connectedWithRoomsState } from '../../__test-utils__';
import OpenGames from './OpenGames';

vi.mock('@app/hooks', async (importOriginal) => {
  const actual = await importOriginal<typeof import('@app/hooks')>();
  return { ...actual, useWebClient: vi.fn(() => ({})) };
});

describe('OpenGames', () => {
  const roomWithGames = {
    info: { roomId: 1, name: 'Main Room' },
  };

  it('renders the games table headers', () => {
    renderWithProviders(<OpenGames room={roomWithGames} />, {
      preloadedState: connectedWithRoomsState,
    });

    expect(screen.getByText('Age')).toBeInTheDocument();
    expect(screen.getByText('Description')).toBeInTheDocument();
    expect(screen.getByText('Creator')).toBeInTheDocument();
    expect(screen.getByText('Type')).toBeInTheDocument();
    expect(screen.getByText('Players')).toBeInTheDocument();
    expect(screen.getByText('Spectators')).toBeInTheDocument();
  });

  it('renders without crashing when no games exist', () => {
    const { container } = renderWithProviders(<OpenGames room={roomWithGames} />, {
      preloadedState: connectedWithRoomsState,
    });

    expect(container.querySelector('.games')).toBeInTheDocument();
  });
});
