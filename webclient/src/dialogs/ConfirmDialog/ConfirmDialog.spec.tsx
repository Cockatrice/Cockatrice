import { screen, fireEvent } from '@testing-library/react';

import { renderWithProviders } from '../../__test-utils__';
import ConfirmDialog from './ConfirmDialog';

describe('ConfirmDialog', () => {
  it('renders the title, message, and default confirm/cancel labels', () => {
    renderWithProviders(
      <ConfirmDialog
        isOpen
        title="Concede this game?"
        message="This can't be undone except by unconcede."
        onConfirm={() => {}}
        onCancel={() => {}}
      />,
    );

    expect(screen.getByText('Concede this game?')).toBeInTheDocument();
    expect(screen.getByText(/can't be undone/i)).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /confirm/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /cancel/i })).toBeInTheDocument();
  });

  it('fires onConfirm when the confirm button is clicked', () => {
    const onConfirm = vi.fn();
    renderWithProviders(
      <ConfirmDialog
        isOpen
        title="T"
        message="M"
        confirmLabel="Concede"
        onConfirm={onConfirm}
        onCancel={() => {}}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /concede/i }));
    expect(onConfirm).toHaveBeenCalled();
  });

  it('fires onCancel when the cancel button is clicked', () => {
    const onCancel = vi.fn();
    renderWithProviders(
      <ConfirmDialog
        isOpen
        title="T"
        message="M"
        onConfirm={() => {}}
        onCancel={onCancel}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));
    expect(onCancel).toHaveBeenCalled();
  });

  it('does not render when closed', () => {
    renderWithProviders(
      <ConfirmDialog
        isOpen={false}
        title="T"
        message="M"
        onConfirm={() => {}}
        onCancel={() => {}}
      />,
    );
    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });
});
