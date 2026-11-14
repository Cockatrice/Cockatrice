## Building Doxygen Documentation

We use [Doxygen](https://www.doxygen.nl) for our code documentation.  
After installing the tool, you can build a HTML webpage into the `/docs-output` folder according to our [configuration](https://github.com/Cockatrice/Cockatrice/blob/master/Doxyfile):
```bash
doxygen Doxyfile
```
Open the `index.html` file in the `/docs-output/html/` folder to browse the result locally.  
There is also an online version we host at the Cockatrice webpage: [cockatrice.github.io/docs](https://cockatrice.github.io/docs)

Doxygen's own documenation can be found at [www.doxygen.nl/manual](https://www.doxygen.nl/manual).

<br>

## Documentation Theme

The used [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) theme requires specific settings in the `Doxyfile` config.  From the [theme docs](https://jothepro.github.io/doxygen-awesome-css/index.html#autotoc_md16):
```
HTML_EXTRA_STYLESHEET  = doxygen-awesome.css    # Required, main CSS file of the theme
HTML_COLORSTYLE        = LIGHT                  # Required, "Auto Dark Mode" will work nontheless
FULL_SIDEBAR           = NO                     # Required
DISABLE_INDEX          = NO                     # Required, YES is bugged in the theme. See jothepro/doxygen-awesome-css/issues/201
GENERATE_TREEVIEW      = YES                    # Optional
```

<br>

Cockatrice dedicated color adjustments are configured in the [cockatrice_docs_style.css](https://github.com/Cockatrice/Cockatrice/blob/master/docs/doxygen/css/cockatrice_docs_style.css) file that uses & overrides the theme definitions.
