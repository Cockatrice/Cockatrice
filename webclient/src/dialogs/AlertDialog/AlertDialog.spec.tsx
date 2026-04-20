import { screen, fireEvent } from '@testing-library/react';

import { renderWithProviders } from '../../__test-utils__';
import AlertDialog from './AlertDialog';

describe('AlertDialog', () => {
  it('renders the title, message, and default OK button', () => {
    renderWithProviders(
      <AlertDialog
        isOpen
        title="Error"
        message="The game is already full."
        onDismiss={() => {}}
      />,
    );

    expect(screen.getByText('Error')).toBeInTheDocument();
    expect(screen.getByText('The game is already full.')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /^ok$/i })).toBeInTheDocument();
  });

  it('uses a custom buttonLabel when provided', () => {
    renderWithProviders(
      <AlertDialog
        isOpen
        title="T"
        message="M"
        buttonLabel="Dismiss"
        onDismiss={() => {}}
      />,
    );
    expect(screen.getByRole('button', { name: /dismiss/i })).toBeInTheDocument();
  });

  it('fires onDismiss when the OK button is clicked', () => {
    const onDismiss = vi.fn();
    renderWithProviders(
      <AlertDialog
        isOpen
        title="T"
        message="M"
        onDismiss={onDismiss}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /^ok$/i }));
    expect(onDismiss).toHaveBeenCalled();
  });

  it('fires onDismiss on Escape key', () => {
    const onDismiss = vi.fn();
    renderWithProviders(
      <AlertDialog
        isOpen
        title="T"
        message="M"
        onDismiss={onDismiss}
      />,
    );

    fireEvent.keyDown(screen.getByRole('dialog'), { key: 'Escape', code: 'Escape' });
    expect(onDismiss).toHaveBeenCalled();
  });

  it('does not render when closed', () => {
    renderWithProviders(
      <AlertDialog
        isOpen={false}
        title="T"
        message="M"
        onDismiss={() => {}}
      />,
    );
    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });
});
