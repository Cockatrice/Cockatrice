#!/bin/sh

# Print the header
echo '<?xml version="1.0" encoding="UTF-8"?>'
echo '<cockatrice_setdatabase version="'`date +%Y%m%d`'">'
echo '<picture_url>http://gatherer.wizards.com/Handlers/Image.ashx?multiverseid=!cardid!&amp;type=card</picture_url>'
echo '<set_url>http://gatherer.wizards.com/Pages/Search/Default.aspx?output=spoiler&amp;method=text&amp;set=[&quot;!longname!&quot;]&amp;special=true</set_url>'

# Download the sets and print them in XML
lynx -dump -source http://magiccards.info/sitemap.html |grep Gatecrash|sed 's/<li><a href="/\n/g'|cut -f1-2 -d\>|grep ^/|sort|sed 's@/en.html">@\t@g'|sed 's@</a@@'|sed 's@^/@@' | awk 'BEGIN{ FS = "\t"}; {print "\t<set import=\"1\">\n\t\t<name>"toupper($1)"</name>\n\t\t<longname>"$2"</longname>\n\t</set>"}'

# Print the footer
echo '</cockatrice_setdatabase>'
