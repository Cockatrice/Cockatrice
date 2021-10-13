// eslint-disable-next-line
export const URL_REGEX = /(https?:\/\/(?:www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,4}\b(?:[-a-zA-Z0-9@:%_\+.~#?&//=]*))/g;
export const MESSAGE_SENDER_REGEX = /(^[^:\s]+):/;
export const MENTION_REGEX = /(^|\s)(@\w+)/g;
export const CARD_CALLOUT_REGEX = /(\[\[[^\]]+\]\])/g;
export const CALLOUT_BOUNDARY_REGEX = /(\[\[|\]\])/g;
