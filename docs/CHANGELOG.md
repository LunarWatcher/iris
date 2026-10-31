---
{
    "title": "Changelog"
}
---
# Changelog

This changelog is meant for people writing templates. For developers embedding iris, see the [API changelog](API changelog.md), which contains developer-specific changes that don't directly affect templates.

{{ page.table_of_contents }}

## Unreleased

## v0.3.0 (2026-10-31)

v0.3.0 is primarily a technical release.

### Added

* Implementations using iris can now produce better errors. Iris itself also produces slightly more useful errors by default by pointing at the source file the error was caused in, if the information is available.
* Varadic functions (largely user-defined functions) now allow 0 arguments to be passed

### Fixed

* Prevent `capitalize` from throwing on an empty string
* Fixed `{% set object.key = "value" %}` assignment

## v0.2.0

### Added

* `set` expressions referencing variables inside an object or array is now legal (i.e. `{% set a = "value" %}{% set x = { "k": a } %}` no longer throws a JSON deserialization error) ([#2](https://codeberg.org/LunarWatcher/iris/issues/2)).
* `set` expressions are now allowed to contain both math and function calls in theri values

### Fixed

* Malformed array expressions like `{% set x = ["a", "b" %}` now throw an understandable error instead of some generic shit ([violet#26](https://codeberg.org/LunarWatcher/violet/issues/26), [#1](https://codeberg.org/LunarWatcher/iris/issues/1))

## v0.1.0

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

