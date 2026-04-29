/**
 * Shared color constants for the game board surface.
 *
 * Two flavors for each named color:
 *   - `*_RGBA` — `{ r, g, b, a }` shape expected by protobuf `color` messages
 *     (arrow color, counter color) and `rgbaToCss()`.
 *   - `*_CSS`  — ready-to-paste CSS string, mirrored by the matching
 *     CSS custom property declared at `:root` in `src/colors.css`.
 *
 * Keep both in sync: add new entries in both the TS constant and the
 * `:root` block, or introduce a generator if the list grows.
 */

export interface ColorRGBA {
  r: number;
  g: number;
  b: number;
  a: number;
}

export const ArrowColor = {
  RED: { r: 224, g: 75, b: 59, a: 255 } as ColorRGBA,
  YELLOW: { r: 240, g: 200, b: 60, a: 255 } as ColorRGBA,
  BLUE: { r: 137, g: 184, b: 224, a: 255 } as ColorRGBA,
  GREEN: { r: 61, g: 162, b: 107, a: 255 } as ColorRGBA,
} as const;

/**
 * Highlight yellow used for the active-turn indicator, the host crown, and
 * the `:focus-visible` outline. Kept as one shade for visual consistency.
 */
export const HIGHLIGHT_YELLOW_CSS = '#f7b01c';

export function rgbaToCss(c: ColorRGBA): string {
  return `rgba(${c.r}, ${c.g}, ${c.b}, ${c.a / 255})`;
}
