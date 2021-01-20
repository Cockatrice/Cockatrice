import $ from "jquery";

export function sanitizeHtml(msg) {
  const $div = $("<div>").html(msg);
  const whitelist = {
    tags: "br,a,img,center,b,font",
    attrs: ["href","color"],
    href: ["http://","https://","ftp://","//"]
  };

  // remove all tags, attributes, and href protocols except some
  enforceTagWhitelist($div, whitelist.tags);
  enforceAttrWhitelist($div, whitelist.attrs);
  enforceHrefWhitelist($div, whitelist.href);

  return $div.html();
}

function enforceTagWhitelist($el, tags) {
  $el.find("*").not(tags).each(function() {
    $(this).replaceWith(this.innerHTML);
  });
}

function enforceAttrWhitelist($el, attrs) {
  $el.find("*").each(function() {
    var attributes = this.attributes;
    var i = attributes.length;
    while( i-- ) {
      var attr = attributes[i];
      if( $.inArray(attr.name,attrs) === -1 )
        this.removeAttributeNode(attr);
    }
  });
}

function enforceHrefWhitelist($el, hrefs) {
  $el.find("[href]").each(function() {
    const $_el = $(this);
    const  attributeValue = $_el.attr("href");

    for (let protocol in hrefs) {
        if (attributeValue.indexOf(hrefs[protocol]) === 0) {
            $_el.attr("target", "_blank");
            return;
        }
    }

    $_el.removeAttr("href");        
  });
}