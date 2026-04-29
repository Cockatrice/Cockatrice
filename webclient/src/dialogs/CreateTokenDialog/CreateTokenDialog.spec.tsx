import { screen, fireEvent } from '@testing-library/react';

import { renderWithProviders } from '../../__test-utils__';
import CreateTokenDialog from './CreateTokenDialog';

describe('CreateTokenDialog', () => {
  it('submits the trimmed name, selected color, P/T, annotation, and flags', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />,
    );

    fireEvent.change(screen.getByLabelText('Token name'), {
      target: { value: '  Goblin  ' },
    });
    fireEvent.change(screen.getByLabelText('Token power/toughness'), {
      target: { value: '1/1' },
    });
    fireEvent.change(screen.getByLabelText('Token annotation'), {
      target: { value: 'ETB' },
    });
    fireEvent.click(screen.getByRole('button', { name: /create/i }));

    // Default color is White ('w') to match desktop DlgCreateToken default.
    expect(onSubmit).toHaveBeenCalledWith({
      name: 'Goblin',
      color: 'w',
      pt: '1/1',
      annotation: 'ETB',
      destroyOnZoneChange: true,
      faceDown: false,
    });
  });

  it('requires a non-empty name', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /create/i }));

    expect(onSubmit).not.toHaveBeenCalled();
    expect(screen.getByText(/name is required/i)).toBeInTheDocument();
  });

  it('caps the name input at the desktop max (255 chars)', () => {
    renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={() => {}} onCancel={() => {}} />,
    );

    const input = screen.getByLabelText('Token name') as HTMLInputElement;
    const longInput = 'x'.repeat(300);
    fireEvent.change(input, { target: { value: longInput } });

    expect(input.value.length).toBeLessThanOrEqual(255);
  });

  it('toggles the destroyOnZoneChange checkbox off when unchecked', () => {
    const onSubmit = vi.fn();
    renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />,
    );

    fireEvent.change(screen.getByLabelText('Token name'), {
      target: { value: 'Persistent' },
    });
    fireEvent.click(
      screen.getByRole('checkbox', { name: /destroy when it leaves the table/i }),
    );
    fireEvent.click(screen.getByRole('button', { name: /create/i }));

    expect(onSubmit).toHaveBeenCalledWith(
      expect.objectContaining({ destroyOnZoneChange: false }),
    );
  });

  it('fires onCancel when Cancel is clicked', () => {
    const onCancel = vi.fn();
    renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={() => {}} onCancel={onCancel} />,
    );

    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));

    expect(onCancel).toHaveBeenCalled();
  });

  it('resets form state when reopened', () => {
    const { rerender } = renderWithProviders(
      <CreateTokenDialog isOpen onSubmit={() => {}} onCancel={() => {}} />,
    );

    const input = screen.getByLabelText('Token name') as HTMLInputElement;
    fireEvent.change(input, { target: { value: 'temp' } });
    expect(input.value).toBe('temp');

    rerender(<CreateTokenDialog isOpen={false} onSubmit={() => {}} onCancel={() => {}} />);
    rerender(<CreateTokenDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);

    const freshInput = screen.getByLabelText('Token name') as HTMLInputElement;
    expect(freshInput.value).toBe('');
  });
});
