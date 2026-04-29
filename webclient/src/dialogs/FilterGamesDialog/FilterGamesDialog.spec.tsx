import { fireEvent, screen } from '@testing-library/react';
import { renderWithProviders } from '../../__test-utils__';
import { DEFAULT_GAME_FILTERS } from '@app/store';
import FilterGamesDialog from './FilterGamesDialog';

function renderDialog(opts: { gametypeMap?: Record<number, string>; initialFilters?: typeof DEFAULT_GAME_FILTERS } = {}) {
  const onSubmit = vi.fn();
  const onCancel = vi.fn();
  renderWithProviders(
    <FilterGamesDialog
      isOpen
      gametypeMap={opts.gametypeMap ?? {}}
      initialFilters={opts.initialFilters ?? DEFAULT_GAME_FILTERS}
      onSubmit={onSubmit}
      onCancel={onCancel}
    />,
  );
  return { onSubmit, onCancel };
}

describe('FilterGamesDialog', () => {
  it('renders the hide-toggles section', () => {
    renderDialog();
    expect(screen.getByLabelText(/Hide full games/i)).toBeInTheDocument();
    expect(screen.getByLabelText(/Hide games that started/i)).toBeInTheDocument();
    expect(screen.getByLabelText(/Hide password-protected games/i)).toBeInTheDocument();
  });

  it('disables spectator sub-filters until "show only if spectators can watch" is checked', () => {
    renderDialog();
    expect(screen.getByLabelText(/spectators need a password/i)).toBeDisabled();
    expect(screen.getByLabelText(/spectators can chat/i)).toBeDisabled();
    expect(screen.getByLabelText(/spectators see hands/i)).toBeDisabled();

    fireEvent.click(screen.getByLabelText(/Show only games where spectators can watch/i));

    expect(screen.getByLabelText(/spectators need a password/i)).not.toBeDisabled();
    expect(screen.getByLabelText(/spectators can chat/i)).not.toBeDisabled();
    expect(screen.getByLabelText(/spectators see hands/i)).not.toBeDisabled();
  });

  it('Apply submits the unchanged defaults when nothing is edited', () => {
    const { onSubmit } = renderDialog();
    fireEvent.click(screen.getByRole('button', { name: /Apply/i }));
    expect(onSubmit).toHaveBeenCalledWith(DEFAULT_GAME_FILTERS);
  });

  it('Apply forwards the toggled hide-full-games filter', () => {
    const { onSubmit } = renderDialog();
    fireEvent.click(screen.getByLabelText(/Hide full games/i));
    fireEvent.click(screen.getByRole('button', { name: /Apply/i }));
    expect(onSubmit).toHaveBeenCalledTimes(1);
    expect(onSubmit.mock.calls[0][0].hideFullGames).toBe(true);
  });

  it('parses the comma-separated creator names into a trimmed list', () => {
    const { onSubmit } = renderDialog();
    fireEvent.change(screen.getByLabelText(/Creator names/i), { target: { value: 'alice, bob ,carol' } });
    fireEvent.click(screen.getByRole('button', { name: /Apply/i }));
    expect(onSubmit.mock.calls[0][0].creatorNameFilters).toEqual(['alice', 'bob', 'carol']);
  });

  it('renders a checkbox per game type and toggles selection', () => {
    const { onSubmit } = renderDialog({ gametypeMap: { 0: 'Constructed', 1: 'Limited' } });
    fireEvent.click(screen.getByLabelText('Constructed'));
    fireEvent.click(screen.getByRole('button', { name: /Apply/i }));
    expect(onSubmit.mock.calls[0][0].gameTypeFilter).toEqual([0]);
  });

  it('Reset restores defaults in the form (Apply submits defaults)', () => {
    const { onSubmit } = renderDialog({
      initialFilters: { ...DEFAULT_GAME_FILTERS, hideFullGames: true, gameNameFilter: 'foo' },
    });
    fireEvent.click(screen.getByRole('button', { name: /Reset/i }));
    fireEvent.click(screen.getByRole('button', { name: /Apply/i }));
    expect(onSubmit).toHaveBeenCalledWith(DEFAULT_GAME_FILTERS);
  });

  it('Cancel calls onCancel', () => {
    const { onCancel } = renderDialog();
    fireEvent.click(screen.getByRole('button', { name: /Cancel/i }));
    expect(onCancel).toHaveBeenCalledTimes(1);
  });
});
