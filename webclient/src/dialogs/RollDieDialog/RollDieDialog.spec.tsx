import { render, screen, fireEvent } from '@testing-library/react';

import RollDieDialog from './RollDieDialog';

describe('RollDieDialog', () => {
  it('does not render when closed', () => {
    render(
      <RollDieDialog isOpen={false} onSubmit={() => {}} onCancel={() => {}} />,
    );
    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('defaults the Sides input to 6 and Count to 1', () => {
    render(<RollDieDialog isOpen onSubmit={() => {}} onCancel={() => {}} />);
    expect((screen.getByLabelText('Sides') as HTMLInputElement).value).toBe('6');
    expect((screen.getByLabelText('Count') as HTMLInputElement).value).toBe('1');
  });

  it('seeds the inputs with the most recent values', () => {
    render(
      <RollDieDialog
        isOpen
        lastSides={20}
        lastCount={4}
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );
    expect((screen.getByLabelText('Sides') as HTMLInputElement).value).toBe('20');
    expect((screen.getByLabelText('Count') as HTMLInputElement).value).toBe('4');
  });

  it('dispatches onSubmit with both sides and count on Roll', () => {
    const onSubmit = vi.fn();
    render(<RollDieDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.change(screen.getByLabelText('Sides'), { target: { value: '6' } });
    fireEvent.change(screen.getByLabelText('Count'), { target: { value: '4' } });
    fireEvent.click(screen.getByRole('button', { name: /^roll$/i }));

    expect(onSubmit).toHaveBeenCalledWith({ sides: 6, count: 4 });
  });

  it('accepts sides = 1 (matches desktop)', () => {
    const onSubmit = vi.fn();
    render(<RollDieDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.change(screen.getByLabelText('Sides'), { target: { value: '1' } });
    fireEvent.click(screen.getByRole('button', { name: /^roll$/i }));

    expect(onSubmit).toHaveBeenCalledWith({ sides: 1, count: 1 });
  });

  it('rejects sides < 1 and surfaces the error on the Sides field', () => {
    const onSubmit = vi.fn();
    render(<RollDieDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.change(screen.getByLabelText('Sides'), { target: { value: '0' } });
    fireEvent.click(screen.getByRole('button', { name: /^roll$/i }));

    expect(onSubmit).not.toHaveBeenCalled();
    expect(screen.getByText(/integer/i)).toBeInTheDocument();
  });

  it('rejects count < 1', () => {
    const onSubmit = vi.fn();
    render(<RollDieDialog isOpen onSubmit={onSubmit} onCancel={() => {}} />);

    fireEvent.change(screen.getByLabelText('Count'), { target: { value: '0' } });
    fireEvent.click(screen.getByRole('button', { name: /^roll$/i }));

    expect(onSubmit).not.toHaveBeenCalled();
  });

  it('dispatches onCancel on Cancel', () => {
    const onCancel = vi.fn();
    render(<RollDieDialog isOpen onSubmit={() => {}} onCancel={onCancel} />);
    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));
    expect(onCancel).toHaveBeenCalled();
  });
});
