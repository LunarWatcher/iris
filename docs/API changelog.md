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

* Added `iris::CallbackError`, which should be thrown by end-user applications in custom callbacks.
* Added an optional argument for adding a filename to templates rendered without a file
  * This is used in error handling to make things clearer
* Added `importStack` and `filePathOrIdentifier` to `SourceLocation`
  * This allows implementing applications to create stacktraces
* Added explicit structured data on whether an `iris::IrisError` is a renderer error, parse error, or general error (happens outside the renderer and parser)
* Added support for vararg functions to accept 0 arguments
  * This is largely going to be useful with the new udata setup
* Added support for rendering with user-defined data. See the docs for custom callbacks for more info.

### Removed

* Removed `parse_template`. Use `parse_file` instead, it's completely identical.

## v0.2.0

### Changed

* The root object passed to the `render` functions must now be an object. This has not been explicitly enforced earlier, but I believe this is only handled indirectly through access into the object causing exceptions in the JSON parsing library (no attempt has been made to verify this because it isn't important).
* BREAKING: iris is no longer header-only. This is not breaking for `FetchContent`-users.
  * This is mostly done to make the code easier to maintain, and to improve compile times. Iris is 4000 lines of code now, which is a fair bit to compile every time iris is included.

### Removed

* BREAKING: `nlomann/json` is no longer used as a dependency.
  * All use of `nlohmann/json` must be replaced with `iris::DataObject`, which has some differing mechanics from nlohmann/json. Notably, objects must be explicitly initialized with `iris::Object`


