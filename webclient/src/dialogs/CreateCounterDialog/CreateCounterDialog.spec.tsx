import { render, screen, fireEvent } from '@testing-library/react';

import CreateCounterDialog from './CreateCounterDialog';

describe('CreateCounterDialog', () => {
  it('does not render when closed', () => {
    render(
      <CreateCounterDialog isOpen={false} onSubmit={() => {}} onCancel={() => {}} />,
    );
    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('renders the name input and 8 color swatches', () => {
    render(<CreateCounterDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);

    expect(screen.getByLabelText('Counter name')).toBeInTheDocument();
    expect(screen.getAllByRole('radio')).toHaveLength(8);
  });

  it('pre-selects the first swatch', () => {
    render(<CreateCounterDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);

    const radios = screen.getAllByRole('radio');
    expect(radios[0]).toHaveAttribute('aria-checked', 'true');
    radios.slice(1).forEach((r) => expect(r).toHaveAttribute('aria-checked', 'false'));
  });

  it('changes selection when a different swatch is clicked', () => {
    render(<CreateCounterDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);

    const red = screen.getByLabelText('Red');
    fireEvent.click(red);
    expect(red).toHaveAttribute('aria-checked', 'true');
  });

  it('requires a non-empty name', () => {
    const onSubmit = vi.fn();
    render(<CreateCounterDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.click(screen.getByRole('button', { name: /^create$/i }));

    expect(onSubmit).not.toHaveBeenCalled();
    expect(screen.getByText(/name is required/i)).toBeInTheDocument();
  });

  it('dispatches onSubmit with the trimmed name and selected color', () => {
    const onSubmit = vi.fn();
    render(<CreateCounterDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.change(screen.getByLabelText('Counter name'), {
      target: { value: '  Poison  ' },
    });
    fireEvent.click(screen.getByLabelText('Green'));
    fireEvent.click(screen.getByRole('button', { name: /^create$/i }));

    expect(onSubmit).toHaveBeenCalledWith({
      name: 'Poison',
      color: { r: 61, g: 162, b: 107, a: 255 },
    });
  });

  it('resets state when the dialog reopens', () => {
    const { rerender } = render(
      <CreateCounterDialog isOpen onSubmit={() => {}} onCancel={() => {}} />,
    );

    fireEvent.change(screen.getByLabelText('Counter name'), { target: { value: 'stale' } });
    fireEvent.click(screen.getByLabelText('Red'));

    rerender(<CreateCounterDialog isOpen={false} onSubmit={() => {}} onCancel={() => {}} />);
    rerender(<CreateCounterDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);

    expect((screen.getByLabelText('Counter name') as HTMLInputElement).value).toBe('');
    expect(screen.getAllByRole('radio')[0]).toHaveAttribute('aria-checked', 'true');
  });

  it('dispatches onCancel on Cancel', () => {
    const onCancel = vi.fn();
    render(<CreateCounterDialog isOpen onSubmit={() => {}} onCancel={onCancel} />);
    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));
    expect(onCancel).toHaveBeenCalled();
  });
});
