import {
  render,
  fireEvent,
  getByRole,
  waitFor,
  act
} from '@testing-library/react';
import { useFireOnce } from './useFireOnce';

describe('useFireOnce hook', () => {
  test('it only fires once when button is clicked twice', async () => {
    // Mock a promise with a delay
    const onClickWithPromise = jest.fn((e) => {
      e.preventDefault()
      return new Promise((resolve) => {
        setTimeout(() => {
          resolve(true);
        }, 100);
      });
    });

    function Button(props) {
      const { children, onClick } = props
      const [buttonIsDisabled, setButtonIsDisabled, handleClickOnce] = useFireOnce(onClick)
      return <button onClick={handleClickOnce} disabled={buttonIsDisabled}>{children}</button>
    }

    // render the button
    const { getByRole } = render(
      <Button onClick={onClickWithPromise}>Click Me!</Button>
    );

    //Grab the button from the DOM and confirm it initialized in an enabled state
    const button = getByRole('button', { name: 'Click Me!' });
    expect(button).toBeEnabled();

    // Simulate two click events in a row
    fireEvent.click(button);
    fireEvent.click(button);

    // Confirm that it's disabled
    await waitFor(() => {
      expect(button).toBeDisabled();
    });

    // Confirm it became enabled after the timeout and that the click event was only fired once
    await waitFor(
      () => {
        expect(onClickWithPromise).toHaveBeenCalledTimes(1);
      },
      { timeout: 100 }
    );
  });

  test('it only fires once when form is submitted twice', async () => {
    // Mock a promise with a delay
    const onClickWithPromise = jest.fn((e) => {
      e.preventDefault()
      return new Promise((resolve) => {
        setTimeout(() => {
          resolve(true);
        }, 100);
      });
    });

    function Form(props) {
      const { onSubmit } = props
      const [buttonIsDisabled, setButtonIsDisabled, handleSubmitOnce] = useFireOnce(onSubmit)
      return (
        <form onSubmit={handleSubmitOnce}>
          <input type="text" defaultValue="Hell World" name="thing-to-say" />
          <button disabled={buttonIsDisabled}>Click Me!</button>
        </form>
      )
    }

    // render the form
    const { getByRole } = render(
      <Form onSubmit={onClickWithPromise} />
    );

    //Grab the button from the DOM and confirm it initialized in an enabled state
    const button = getByRole('button', { name: 'Click Me!' });
    expect(button).toBeEnabled();

    // Simulate two click events in a row
    fireEvent.click(button);
    fireEvent.click(button);

    // Confirm that it's disabled
    await waitFor(() => {
      expect(button).toBeDisabled();
    });

    // Confirm it became enabled after the timeout and that the click event was only fired once
    await waitFor(
      () => {
        expect(onClickWithPromise).toHaveBeenCalledTimes(1);
      },
      { timeout: 100 }
    );
  });
});
