import { screen, fireEvent } from '@testing-library/react';

import { createMockWebClient, renderWithProviders } from '../../../__test-utils__';
import HandContextMenu from './HandContextMenu';

function render(overrides: Partial<React.ComponentProps<typeof HandContextMenu>> = {}) {
  const props: React.ComponentProps<typeof HandContextMenu> = {
    isOpen: true,
    anchorPosition: { top: 10, left: 10 },
    gameId: 1,
    handSize: 7,
    onClose: vi.fn(),
    onRequestChooseMulligan: vi.fn(),
    onRequestRevealHand: vi.fn(),
    onRequestRevealRandom: vi.fn(),
    ...overrides,
  };
  const webClient = createMockWebClient();
  return {
    ...renderWithProviders(<HandContextMenu {...props} />, { webClient }),
    webClient,
    props,
  };
}

describe('HandContextMenu', () => {
  it('fires onRequestChooseMulligan and closes when the choose-size item is clicked', () => {
    const onRequestChooseMulligan = vi.fn();
    const onClose = vi.fn();
    render({ onRequestChooseMulligan, onClose });

    fireEvent.click(screen.getByRole('menuitem', { name: /choose size/i }));

    expect(onRequestChooseMulligan).toHaveBeenCalled();
    expect(onClose).toHaveBeenCalled();
  });

  it('dispatches mulligan(number=handSize) on the same-size item', () => {
    const { webClient } = render({ handSize: 7 });

    fireEvent.click(screen.getByRole('menuitem', { name: /same size/i }));

    expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 7 });
  });

  it('dispatches mulligan(number=handSize-1) on the size−1 item', () => {
    const { webClient } = render({ handSize: 5 });

    fireEvent.click(screen.getByRole('menuitem', { name: /size − 1/i }));

    expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 4 });
  });

  it('floors size−1 at 1, matching desktop actMulliganMinusOne', () => {
    const { webClient } = render({ handSize: 1 });

    fireEvent.click(screen.getByRole('menuitem', { name: /size − 1/i }));

    expect(webClient.request.game.mulligan).toHaveBeenCalledWith(1, { number: 1 });
  });

  it('disables same-size when handSize is 0', () => {
    render({ handSize: 0 });

    expect(screen.getByRole('menuitem', { name: /same size/i })).toHaveAttribute(
      'aria-disabled',
      'true',
    );
  });

  it('fires onRequestRevealHand and closes on reveal-hand item', () => {
    const onRequestRevealHand = vi.fn();
    const onClose = vi.fn();
    render({ onRequestRevealHand, onClose });

    fireEvent.click(screen.getByRole('menuitem', { name: /reveal hand/i }));

    expect(onRequestRevealHand).toHaveBeenCalled();
    expect(onClose).toHaveBeenCalled();
  });
});
