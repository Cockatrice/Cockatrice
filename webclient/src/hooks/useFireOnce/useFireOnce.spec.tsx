import {
  render,
  fireEvent,
  waitFor,
  renderHook,
  act,
} from '@testing-library/react';
import { useFireOnce } from './useFireOnce';

describe('useFireOnce hook', () => {
  test('it only fires once when button is clicked twice', async () => {
    const onClickWithPromise = vi.fn((e) => {
      e.preventDefault()
      return new Promise((resolve) => {
        setTimeout(() => {
          resolve(true);
        }, 100);
      });
    });

    function Button(props) {
      const { children, onClick } = props
      const [buttonIsDisabled, _setButtonIsDisabled, handleClickOnce] = useFireOnce(onClick)
      return <button onClick={handleClickOnce} disabled={buttonIsDisabled}>{children}</button>
    }

    const { getByRole } = render(
      <Button onClick={onClickWithPromise}>Click Me!</Button>
    );

    const button = getByRole('button', { name: 'Click Me!' });
    expect(button).toBeEnabled();

    fireEvent.click(button);
    fireEvent.click(button);

    await waitFor(() => {
      expect(button).toBeDisabled();
    });

    await waitFor(
      () => {
        expect(onClickWithPromise).toHaveBeenCalledTimes(1);
      },
      { timeout: 100 }
    );
  });

  test('it only fires once when form is submitted twice', async () => {
    const onClickWithPromise = vi.fn((e) => {
      e.preventDefault()
      return new Promise((resolve) => {
        setTimeout(() => {
          resolve(true);
        }, 100);
      });
    });

    function Form(props) {
      const { onSubmit } = props
      const [buttonIsDisabled, _setButtonIsDisabled, handleSubmitOnce] = useFireOnce(onSubmit)
      return (
        <form onSubmit={handleSubmitOnce}>
          <input type="text" defaultValue="Hell World" name="thing-to-say" />
          <button disabled={buttonIsDisabled}>Click Me!</button>
        </form>
      )
    }

    const { getByRole } = render(
      <Form onSubmit={onClickWithPromise} />
    );

    const button = getByRole('button', { name: 'Click Me!' });
    expect(button).toBeEnabled();

    fireEvent.click(button);
    fireEvent.click(button);

    await waitFor(() => {
      expect(button).toBeDisabled();
    });

    await waitFor(
      () => {
        expect(onClickWithPromise).toHaveBeenCalledTimes(1);
      },
      { timeout: 100 }
    );
  });

  test('resetInFlightStatus re-enables firing', () => {
    const fn = vi.fn();
    const { result } = renderHook(() => useFireOnce(fn));

    act(() => {
      result.current[2]();
    });

    expect(result.current[0]).toBe(true);
    expect(fn).toHaveBeenCalledTimes(1);

    act(() => {
      result.current[1]();
    });

    expect(result.current[0]).toBe(false);

    act(() => {
      result.current[2]();
    });

    expect(fn).toHaveBeenCalledTimes(2);
  });

  test('calls the latest fn when parent updates it', () => {
    const fn1 = vi.fn();
    const fn2 = vi.fn();
    const { result, rerender } = renderHook(({ fn }) => useFireOnce(fn), {
      initialProps: { fn: fn1 },
    });

    rerender({ fn: fn2 });

    act(() => {
      result.current[2]();
    });

    expect(fn1).not.toHaveBeenCalled();
    expect(fn2).toHaveBeenCalledTimes(1);
  });

  test('passes all arguments through to fn', () => {
    const fn = vi.fn();
    const { result } = renderHook(() => useFireOnce(fn));

    act(() => {
      result.current[2]('a', 'b', 'c');
    });

    expect(fn).toHaveBeenCalledWith('a', 'b', 'c');
  });
});
