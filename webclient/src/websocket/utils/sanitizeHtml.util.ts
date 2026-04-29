import DOMPurify from 'dompurify';

DOMPurify.addHook('afterSanitizeAttributes', (node) => {
  if (node.tagName === 'A') {
    node.setAttribute('target', '_blank');
    node.setAttribute('rel', 'noopener noreferrer');
  }
});

export function sanitizeHtml(msg: string): string {
  // Desktop Cockatrice renders MOTD via Qt QTextBrowser with no sanitization;
  // web client hardens via a DOMPurify tag/attr allowlist and restricts URIs
  // to https/http (ftp is effectively dead in modern browsers and would only
  // broaden the attack surface for a hostile server).
  return DOMPurify.sanitize(msg, {
    ALLOWED_TAGS: ['br', 'a', 'img', 'center', 'b', 'font'],
    ALLOWED_ATTR: ['href', 'color', 'rel', 'target', 'src', 'alt'],
    // Load-bearing: DOMPurify applies ALLOWED_URI_REGEXP to *every* attribute
    // it isn't explicitly told is URI-safe. `color` values like "red" don't
    // match the regex and would be stripped. This whitelists `color` out of
    // URI validation. Removing it breaks the <font color="red"> sanitizer test.
    ADD_URI_SAFE_ATTR: ['color'],
    ALLOWED_URI_REGEXP: /^https?:/i,
  });
}
