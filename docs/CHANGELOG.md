---
{
    "title": "Changelog"
}
---
# Changelog

## [unreleased]

## [v0.1.0]

Initial post-fork commit.

No functional changes have been made to the core library. Some changes have been made to distribution and internals.

* Changed the name to iris
* Removed doxygen in favour of violet
* Removed meson support
  * I cannot maintain a whole extra build system. If you need meson and you're willing to help maintain it, feel free to make a PR re-adding support for it.
* Had to throw out the entire test suite, as it appears to use a non-FOSS license. All the files have a copyright header that makes no mention of the MIT license, which is also inconsistent with all the other files.
  * A base setup has been set up to make sure it still compiles and vaguely functions.
* Dropped single-header support. V1 will not be header-only either.

Migration guide:
1. Point FetchContent to `https://codeberg.org/LunarWatcher/iris`.
2. Migrate deps. Automated script:
  ```
  rg 'inja' --files-with-matches | xargs sed -i 's/inja/iris/g'
  # The previous replacement breaks "jinja", so unfuck it. Could in theory use a better regex, but it's good enough
  # These were also used to migrate this repo, so they're tested
  rg 'jiris' --files-with-matches | xargs sed -i 's/jiris/jinja/g'
  ```


[unreleased]: https://codeberg.org/LunarWatcher/iris/compare/v0.1.0...master
<!-- [v0.4.0]: https://codeberg.org/LunarWatcher/iris/compare/v0.3.0...v0.4.0 -->
[v0.1.0]: https://codeberg.org/LunarWatcher/iris/releases/tag/v0.1.0
