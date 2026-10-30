---
{
    "title": "API changelog"
}
---
# API changelog

This page contains the changelog for the iris API. Not all releases will have an entry in this changelog.

This changelog is meant for developers that embed iris in their software. For the changelog that concerns the actual templating language, see [the other changelog](CHANGELOG.md).

{{ page.table_of_contents }}

## Unreleased

## v0.3.0

### Added

* `iris::CallbackError`, which should be thrown by end-user applications in custom callbacks.

## v0.2.0

### Changed

* The root object passed to the `render` functions must now be an object. This has not been explicitly enforced earlier, but I believe this is only handled indirectly through access into the object causing exceptions in the JSON parsing library (no attempt has been made to verify this because it isn't important).
* BREAKING: iris is no longer header-only. This is not breaking for `FetchContent`-users.
  * This is mostly done to make the code easier to maintain, and to improve compile times. Iris is 4000 lines of code now, which is a fair bit to compile every time iris is included.

### Removed

* BREAKING: `nlomann/json` is no longer used as a dependency.
  * All use of `nlohmann/json` must be replaced with `iris::DataObject`, which has some differing mechanics from nlohmann/json. Notably, objects must be explicitly initialized with `iris::Object`


