import $ from 'jquery';

export function sanitizeHtml(msg: string): string {
  const $div = $('<div>').html(msg);
  const whitelist = {
    tags: 'br,a,img,center,b,font',
    attrs: ['href', 'color'],
    href: ['http://', 'https://', 'ftp://', '//']
  };

  // remove all tags, attributes, and href protocols except some
  enforceTagWhitelist($div, whitelist.tags);
  enforceAttrWhitelist($div, whitelist.attrs);
  enforceHrefWhitelist($div, whitelist.href);

  return $div.html();
}

function enforceTagWhitelist($el: JQuery<HTMLElement>, tags: string): void {
  $el.find('*').not(tags).each(function enforceTag() {
    $(this).replaceWith(this.innerHTML);
  });
}

function enforceAttrWhitelist($el: JQuery<HTMLElement>, attrs: string[]): void {
  $el.find('*').each(function enforceAttribute() {
    const attributes = this.attributes;
    let i = attributes.length;
    while (i--) {
      const attr = attributes[i];
      if ($.inArray(attr.name, attrs) === -1) {
        this.removeAttributeNode(attr);
      }
    }
  });
}

function enforceHrefWhitelist($el: JQuery<HTMLElement>, hrefs: string[]): void {
  $el.find('[href]').each(function enforceHref() {
    const $_el = $(this);
    const attributeValue = $_el.attr('href');

    for (let protocol in hrefs) {
      if (attributeValue.indexOf(hrefs[protocol]) === 0) {
        $_el.attr('target', '_blank');
        return;
      }
    }

    $_el.removeAttr('href');
  });
}
