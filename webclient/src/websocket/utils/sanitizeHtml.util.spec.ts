import { sanitizeHtml } from './sanitizeHtml.util';

describe('sanitizeHtml', () => {
  it('passes through plain text unchanged', () => {
    expect(sanitizeHtml('hello world')).toBe('hello world');
  });

  it('allows <br> tag', () => {
    expect(sanitizeHtml('line1<br>line2')).toBe('line1<br>line2');
  });

  it('allows <b> tag', () => {
    expect(sanitizeHtml('<b>bold</b>')).toBe('<b>bold</b>');
  });

  it('allows <img> tag', () => {
    expect(sanitizeHtml('<img>')).toBe('<img>');
  });

  it('allows <center> tag', () => {
    expect(sanitizeHtml('<center>x</center>')).toBe('<center>x</center>');
  });

  it('allows <font> tag with color attribute', () => {
    expect(sanitizeHtml('<font color="red">text</font>')).toBe('<font color="red">text</font>');
  });

  it('strips disallowed tag <script>', () => {
    expect(sanitizeHtml('<script>alert(1)</script>')).toBe('');
  });

  it('strips disallowed tag <div>', () => {
    expect(sanitizeHtml('<div>content</div>')).toBe('content');
  });

  it('strips disallowed attribute onclick from <b>', () => {
    expect(sanitizeHtml('<b onclick="evil()">hi</b>')).toBe('<b>hi</b>');
  });

  it('adds target=_blank and rel=noopener noreferrer to <a> tags', () => {
    const result = sanitizeHtml('<a href="https://example.com">link</a>');
    expect(result).toContain('target="_blank"');
    expect(result).toContain('rel="noopener noreferrer"');
  });

  it('allows href attribute on <a>', () => {
    const result = sanitizeHtml('<a href="https://example.com">link</a>');
    expect(result).toContain('href="https://example.com"');
  });

  it('strips disallowed schemes like javascript:', () => {
    const result = sanitizeHtml('<a href="javascript:alert(1)">xss</a>');
    expect(result).not.toContain('javascript:');
  });

  it('preserves src and alt on img tags', () => {
    const result = sanitizeHtml('<img src="http://example.com/img.png" alt="test" />');
    expect(result).toContain('src="http://example.com/img.png"');
    expect(result).toContain('alt="test"');
  });

  it('strips javascript: scheme from img src', () => {
    const result = sanitizeHtml('<img src="javascript:alert(1)" />');
    expect(result).not.toContain('src="javascript:');
  });

  it('strips ftp: scheme from img src (scheme-hardening vs desktop)', () => {
    const result = sanitizeHtml('<img src="ftp://evil.example/tracker.png" />');
    expect(result).not.toContain('ftp://');
  });

  it('preserves https: scheme on img src', () => {
    const result = sanitizeHtml('<img src="https://example.com/img.png" />');
    expect(result).toContain('src="https://example.com/img.png"');
  });

  it('strips onerror from img while keeping safe src', () => {
    const result = sanitizeHtml('<img src="http://example.com/img.png" onerror="alert(1)" />');
    expect(result).not.toContain('onerror');
    expect(result).toContain('src="http://example.com/img.png"');
  });
});
