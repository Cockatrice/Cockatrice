Add Sets
===

The oracle program creates the cockatrice card database and needs a set database to achieve this.
The set database is written in XML and has the following skeleton:

```
<?xml version="1.0" encoding="UTF-8"?>
<cockatrice_setdatabase version="20100416">
	<picture_url>http://www.wizards.com/global/images/magic/general/%1.jpg</picture_url>
	<set_url>http://gatherer.wizards.com/Pages/Search/Default.aspx?output=spoiler&amp;method=text&amp;set=[&quot;!longname!&quot;]</set_url>

	<set import="1">
		<name>Set Code</name>
		<longname>Set Name</longname>
	</set>

	<set import="1">
		<name>Another Set Code</name>
		<longname>Another Set Name</longname>
	</set>

	...

</cockatrice_setdatabase>
```

The full set database file called "set.xml" should be included in the cockatrice git-sources.

* You could change the picture_url to another source if you want, e.g. if you have HQ scans or another alternative server.
* The set_url is the search URL for Wizard's Gatherer, where the oracle program will fetch it's information from.
* If the Gatherer changes, adapt the URL and keep in mind that '&', '"' and other special characters should be HTML encoded (like '&amp;' and '&quot;').
* Further, the oracle replaces substrings included in '!' characters with information from the set entries, like '!longname!' with the set name.
* If you add a new set to the database, you have to create a new set entry like
	<set import="1">
		<name>AVR</name>
		<longname>Avacyn Restored</longname>
	</set>
and update the date in the version string. If you set import="1" the set's checkbox in the oracle's GUI will be set, otherwise unset.


Here is a more detailed step by step manual found in the internet:

1. Load the Oracle program that's installed with Cockatrice. The Oracle program can usually be found by click the Start button and going to All Programs > Cockatrice.
2. Click the File menu item and select 'Download sets information...'
3. Copy the URL from the 'Load sets' screen (the URL should be something like http://www.cockatrice.de/files/sets.xml) and paste it into your internet browser.
4. Right click on the web page and select Save As
5. Save the XML file to somewhere on your PC, preferably somewhere that's easy to get to
6. Right click on the XML file and select 'Open With...' and choose Notepad from the list
7. Before the end of the notepad document (should be something like </cockatrice_set database>) add:
    <set import="1">
	<name>RTR</name>
	<longname>Return to Ravnica</longname>
    </set>
8. Save and close the XML file
9. Back in the Oracle progam if the 'Load Sets' screen is still open click the Cancel button
10. Click the File menu item and click 'Load Sets from File'
11. Browse to where you saved the XML file and double click on it to select it
12. Click the Start Download button
13. Once the download is complete close the Oracle program and run Cockatrice. The Return to Ravnica cards should now be displayed.

Hopefully the steps above are helpful and easy to follow.