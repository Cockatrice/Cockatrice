import { render } from '@testing-library/react';
import VirtualList from './VirtualList';

describe('VirtualList', () => {
  it('renders without crashing with empty items', () => {
    const { container } = render(<VirtualList items={[]} />);
    expect(container.querySelector('.virtual-list')).toBeInTheDocument();
  });

  it('accepts className as a string', () => {
    const { container } = render(<VirtualList items={[]} className="custom-class" />);
    expect(container.querySelector('.custom-class')).toBeInTheDocument();
  });

  it('applies empty string as default className (not object)', () => {
    const { container } = render(<VirtualList items={[]} />);
    const list = container.querySelector('.virtual-list__list');
    // className should not contain "[object Object]"
    expect(list?.className).not.toContain('[object Object]');
  });
});
