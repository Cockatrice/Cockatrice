## Doxygen Documentation Theme

Required changes to the `Doxyfile` config from the [theme docs](https://jothepro.github.io/doxygen-awesome-css/index.html#autotoc_md16) to make [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) work:
```
HTML_EXTRA_STYLESHEET  = doxygen-awesome.css      # Main CSS file of the theme
GENERATE_TREEVIEW      = YES                      # Optional, also works without
HTML_COLORSTYLE        = LIGHT                    # Required with doxygen-awesome-css theme, Auto Dark Mode will still work
DISABLE_INDEX          = NO                       # YES is bugged in the theme, see jothepro/doxygen-awesome-css/issues/201
FULL_SIDEBAR           = NO                       # Required for doxygen-awesome-css theme
```

<br>

Cockatrice dedicated color adjustments are configured in the [cockatrice_docs_style.css](https://github.com/Cockatrice/Cockatrice/blob/master/doc/doxygen/css/cockatrice_docs_style.css) file that uses & overrides the theme definitions.
