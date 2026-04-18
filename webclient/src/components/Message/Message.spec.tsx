import { screen } from '@testing-library/react';
import { renderWithProviders } from '../../__test-utils__';
import Message from './Message';

describe('Message', () => {
  it('renders a plain message', () => {
    const message = { message: 'Hello world' };
    renderWithProviders(<Message message={message} />);

    expect(screen.getByText('Hello world')).toBeInTheDocument();
  });

  it('renders the message container', () => {
    const message = { message: 'Test message' };
    const { container } = renderWithProviders(<Message message={message} />);

    expect(container.querySelector('.message')).toBeInTheDocument();
  });
});
