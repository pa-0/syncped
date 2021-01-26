# How to contribute

- fork the repository on GitLab

- create a feature branch from the master branch to base your work

## Adding functionality

- use style as indicated by .clang-format

- use STL whenever possible

- icons and bitmaps
  - menu and toolbar bitmaps are from wxWidgets, using wxArtProvider

  - application icons from:
  [IconFinder](https://www.iconfinder.com/icons/1495216/article_circle_edit_paper_pencil_icon#size=128),
  [converted to ico](http://www.convertico.com/),
  [converted to xpm using GIMP](http://www.gimp.org/),
  [convert to mac icns (first make 128 by 128 icon)](http://iconverticons.com/)

- translation is done using xgettext by invoking po-sync.sh
  - to translate wex copy the wex.pot file to the correct language po
    file, and fill in the translation.

  - The place where to put your po files can be found by adding -l command line switch.
    You can also test other languages using the special LANG config item,
    e.g. setting it to french allows you to test french translation.
    Normally you check the current locale by running locale on the
    command line. Running locale -a shows all your available locales, if your
    locale is not present, add it using locale-gen (provided it is in
    the list of all locales (/usr/share/i18n/SUPPORTED).
    Then you do export LANG=..., or change the /etc/default/locale.

  - To add translation files add -DENABLE_GETTEXT=ON to cmake.

  - Do a pull request from the feature branch to the master branch

## ctags osx
- see external/ctags/docs/osx.rst:

```bash
brew tap universal-ctags/universal-ctags
brew install --HEAD universal-ctags
brew reinstall universal-ctags
```
