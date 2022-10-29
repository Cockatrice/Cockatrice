import sanitize from 'sanitize-html';

export function sanitizeHtml(msg: string): string {
  return sanitize(msg, {
    allowedTags: ['br', 'a', 'img', 'center', 'b', 'font'],
    allowedAttributes: {
      '*': ['href', 'color', 'target'],
    },
    allowedSchemes: ['http', 'https', 'ftp'],
    transformTags: {
      'a': sanitize.simpleTransform('a', { target: '_blank' }),
    }
  });
}
