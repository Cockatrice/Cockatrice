import { render, screen, fireEvent } from '@testing-library/react';

import PromptDialog from './PromptDialog';

describe('PromptDialog', () => {
  it('does not render content when closed', () => {
    render(
      <PromptDialog
        isOpen={false}
        title="Set P/T"
        label="P/T"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    expect(screen.queryByRole('dialog')).not.toBeInTheDocument();
  });

  it('renders title, label, OK, Cancel when open', () => {
    render(
      <PromptDialog
        isOpen
        title="Set P/T"
        label="P/T"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    expect(screen.getByText('Set P/T')).toBeInTheDocument();
    expect(screen.getByLabelText('P/T')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /ok/i })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: /cancel/i })).toBeInTheDocument();
  });

  it('seeds the input with initialValue', () => {
    render(
      <PromptDialog
        isOpen
        title="Edit"
        label="Value"
        initialValue="42"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    const input = screen.getByLabelText('Value') as HTMLInputElement;
    expect(input.value).toBe('42');
  });

  it('dispatches onSubmit with the typed value on submit', () => {
    const onSubmit = vi.fn();
    render(
      <PromptDialog
        isOpen
        title="Draw N"
        label="Cards"
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    fireEvent.change(screen.getByLabelText('Cards'), { target: { value: '7' } });
    fireEvent.click(screen.getByRole('button', { name: /ok/i }));

    expect(onSubmit).toHaveBeenCalledWith('7');
  });

  it('dispatches onCancel when Cancel is clicked', () => {
    const onCancel = vi.fn();
    render(
      <PromptDialog
        isOpen
        title="Draw N"
        label="Cards"
        onSubmit={() => {}}
        onCancel={onCancel}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /cancel/i }));

    expect(onCancel).toHaveBeenCalled();
  });

  it('honors a custom submitLabel', () => {
    render(
      <PromptDialog
        isOpen
        title="Edit"
        label="Value"
        submitLabel="Save"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    expect(screen.getByRole('button', { name: /save/i })).toBeInTheDocument();
  });

  it('blocks submit and shows the validator error when validation fails', () => {
    const onSubmit = vi.fn();
    const validate = (v: string) => (v === '' ? 'required' : null);
    render(
      <PromptDialog
        isOpen
        title="Draw N"
        label="Cards"
        validate={validate}
        onSubmit={onSubmit}
        onCancel={() => {}}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /ok/i }));

    expect(onSubmit).not.toHaveBeenCalled();
    expect(screen.getByText('required')).toBeInTheDocument();
  });

  it('clears the validator error once the user starts typing again', () => {
    const validate = (v: string) => (v === '' ? 'required' : null);
    render(
      <PromptDialog
        isOpen
        title="Draw N"
        label="Cards"
        validate={validate}
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    fireEvent.click(screen.getByRole('button', { name: /ok/i }));
    expect(screen.getByText('required')).toBeInTheDocument();

    fireEvent.change(screen.getByLabelText('Cards'), { target: { value: '3' } });
    expect(screen.queryByText('required')).not.toBeInTheDocument();
  });

  it('shows persistent helperText when there is no validation error', () => {
    render(
      <PromptDialog
        isOpen
        title="Take mulligan"
        label="New hand size"
        helperText="0 and lower are in comparison to current hand size."
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );
    expect(
      screen.getByText('0 and lower are in comparison to current hand size.'),
    ).toBeInTheDocument();
  });

  it('replaces helperText with the validator error when validation fails', () => {
    const validate = (v: string) => (v === '' ? 'required' : null);
    render(
      <PromptDialog
        isOpen
        title="Take mulligan"
        label="New hand size"
        helperText="0 and lower are in comparison to current hand size."
        validate={validate}
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );
    fireEvent.click(screen.getByRole('button', { name: /ok/i }));
    expect(screen.getByText('required')).toBeInTheDocument();
    expect(
      screen.queryByText('0 and lower are in comparison to current hand size.'),
    ).not.toBeInTheDocument();
  });

  it('resets the value when reopened with a new initialValue', () => {
    const { rerender } = render(
      <PromptDialog
        isOpen={false}
        title="Edit"
        label="V"
        initialValue="old"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    rerender(
      <PromptDialog
        isOpen
        title="Edit"
        label="V"
        initialValue="new"
        onSubmit={() => {}}
        onCancel={() => {}}
      />,
    );

    const input = screen.getByLabelText('V') as HTMLInputElement;
    expect(input.value).toBe('new');
  });
});
