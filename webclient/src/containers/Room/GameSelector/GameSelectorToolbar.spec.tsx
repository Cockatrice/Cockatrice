import { fireEvent, screen } from '@testing-library/react';
import { renderWithProviders } from '../../../__test-utils__';
import GameSelectorToolbar, { GameSelectorToolbarProps } from './GameSelectorToolbar';

function defaultProps(overrides: Partial<GameSelectorToolbarProps> = {}): GameSelectorToolbarProps {
  return {
    isFilterActive: false,
    canCreate: true,
    canJoin: true,
    canSpectate: true,
    isJudgeUser: false,
    onFilter: vi.fn(),
    onClearFilter: vi.fn(),
    onCreate: vi.fn(),
    onJoin: vi.fn(),
    onSpectate: vi.fn(),
    onJoinAsJudge: vi.fn(),
    onSpectateAsJudge: vi.fn(),
    ...overrides,
  };
}

describe('GameSelectorToolbar', () => {
  it('renders the five always-visible buttons', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps()} />);
    expect(screen.getByRole('button', { name: /Filter games/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /Clear filter/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^Create$/ })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^Join$/ })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /Join as Spectator/i })).toBeInTheDocument();
  });

  it('hides the two judge buttons when isJudgeUser is false', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ isJudgeUser: false })} />);
    expect(screen.queryByRole('button', { name: /Join as Judge$/i })).not.toBeInTheDocument();
    expect(screen.queryByRole('button', { name: /Join as Judge Spectator/i })).not.toBeInTheDocument();
  });

  it('shows the two judge buttons when isJudgeUser is true', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ isJudgeUser: true })} />);
    expect(screen.getByRole('button', { name: /Join as Judge$/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /Join as Judge Spectator/i })).toBeInTheDocument();
  });

  it('disables Clear filter when no filter is active', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ isFilterActive: false })} />);
    expect(screen.getByRole('button', { name: /Clear filter/i })).toBeDisabled();
  });

  it('enables Clear filter when a filter is active', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ isFilterActive: true })} />);
    expect(screen.getByRole('button', { name: /Clear filter/i })).not.toBeDisabled();
  });

  it('disables Join when canJoin is false', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ canJoin: false })} />);
    expect(screen.getByRole('button', { name: /^Join$/ })).toBeDisabled();
  });

  it('disables Join as Spectator when canSpectate is false', () => {
    renderWithProviders(<GameSelectorToolbar {...defaultProps({ canSpectate: false })} />);
    expect(screen.getByRole('button', { name: /Join as Spectator/i })).toBeDisabled();
  });

  it('judge buttons inherit canJoin / canSpectate gating', () => {
    renderWithProviders(
      <GameSelectorToolbar {...defaultProps({ isJudgeUser: true, canJoin: false, canSpectate: false })} />,
    );
    expect(screen.getByRole('button', { name: /Join as Judge$/i })).toBeDisabled();
    expect(screen.getByRole('button', { name: /Join as Judge Spectator/i })).toBeDisabled();
  });

  it('invokes the corresponding callback for each button', () => {
    const props = defaultProps({ isJudgeUser: true, isFilterActive: true });
    renderWithProviders(<GameSelectorToolbar {...props} />);
    fireEvent.click(screen.getByRole('button', { name: /Filter games/i }));
    fireEvent.click(screen.getByRole('button', { name: /Clear filter/i }));
    fireEvent.click(screen.getByRole('button', { name: /^Create$/ }));
    fireEvent.click(screen.getByRole('button', { name: /^Join$/ }));
    fireEvent.click(screen.getByRole('button', { name: /Join as Spectator/i }));
    fireEvent.click(screen.getByRole('button', { name: /Join as Judge$/i }));
    fireEvent.click(screen.getByRole('button', { name: /Join as Judge Spectator/i }));
    expect(props.onFilter).toHaveBeenCalledTimes(1);
    expect(props.onClearFilter).toHaveBeenCalledTimes(1);
    expect(props.onCreate).toHaveBeenCalledTimes(1);
    expect(props.onJoin).toHaveBeenCalledTimes(1);
    expect(props.onSpectate).toHaveBeenCalledTimes(1);
    expect(props.onJoinAsJudge).toHaveBeenCalledTimes(1);
    expect(props.onSpectateAsJudge).toHaveBeenCalledTimes(1);
  });
});
