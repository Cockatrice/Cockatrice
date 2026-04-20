import { screen, fireEvent } from '@testing-library/react';

import { renderWithProviders } from '../../../__test-utils__';
import PlayerContextMenu from './PlayerContextMenu';

const NOOP = () => {};
const DEFAULT_PROPS = {
  isOpen: true,
  anchorPosition: { top: 10, left: 10 },
  onClose: NOOP,
  onRequestCreateToken: NOOP,
  onRequestViewSideboard: NOOP,
};

describe('PlayerContextMenu', () => {
  it('fires onRequestCreateToken and closes when "Create token…" is clicked', () => {
    const onRequestCreateToken = vi.fn();
    const onClose = vi.fn();

    renderWithProviders(
      <PlayerContextMenu
        {...DEFAULT_PROPS}
        onClose={onClose}
        onRequestCreateToken={onRequestCreateToken}
      />,
    );

    fireEvent.click(screen.getByRole('menuitem', { name: /create token/i }));

    expect(onRequestCreateToken).toHaveBeenCalled();
    expect(onClose).toHaveBeenCalled();
  });

  it('fires onRequestViewSideboard and closes when "View sideboard…" is clicked', () => {
    const onRequestViewSideboard = vi.fn();
    const onClose = vi.fn();

    renderWithProviders(
      <PlayerContextMenu
        {...DEFAULT_PROPS}
        onClose={onClose}
        onRequestViewSideboard={onRequestViewSideboard}
      />,
    );

    fireEvent.click(screen.getByRole('menuitem', { name: /view sideboard/i }));

    expect(onRequestViewSideboard).toHaveBeenCalled();
    expect(onClose).toHaveBeenCalled();
  });

  it('does not render menu items when closed', () => {
    renderWithProviders(
      <PlayerContextMenu
        {...DEFAULT_PROPS}
        isOpen={false}
        anchorPosition={null}
      />,
    );

    expect(screen.queryByRole('menuitem')).not.toBeInTheDocument();
  });
});
