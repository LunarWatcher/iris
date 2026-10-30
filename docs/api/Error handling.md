---
{
    "title": "Error handling"
}
---
# Error handling

Iris offers a few ways of dealing with errors. All iris exceptions inherit from `iris::IrisError`. It contains the following fields:

* `type`: Use is discouraged. Broad type category
* `message`: The error message
* `errType`: Identifies whether the error is a Render error, a Parse error, or a General error. A Render error is an error that happens during a render, a Parse error is an error that happens during parsing, and a General error is an error that happens outside this. This can often be caused by either a developer error, or unexpected errors in the system.
* `location`:  Information about where the error happened. This is defined for `errType != iris::ErrorType::General`
  * `line`: the line the error happened on
  * `column`: the column the error happened on
  * `filePathOrIdentifier`: Either the full path to the file (for all included files, and all files rendered from a file), or a user-provided identifier that may or may not be a file, empty string, or other identifier defined by the user. 

    This is the argument passed to `render()` when not doing file-based rendering.
  * `importStack`: A stack that shows the full include order. This is only defined if `errType == iris::ErrorType::Render`. This is a stack of other SourceLocations, and these SourceLocations point at where the includes happened. SourceLocations in this stack do not have their own `importStacks`; only `line`, `column`, and `filePathOrIdentifier` are defined.

    Note that at parse time, includes and similar are not evaluated. This means that a template that has invalid syntax is invalid regardless of how it was included, so the import stack does not make sense.
    
`iris::IrisError` Contains enough information to produce full compiler-grade error messages. However, this functionality is not included in iris. There are too many use-cases to account for to create a proper stacktrace generator, especially if it's desirable to show output in the terminal that isn't just logging in a service.

Consequently, not all this information will be useful to all error handling methods and all applications. This is also complicated by `filePathOrIdentifier` not necessarily being a file in every application, though it may be internally within an application. Standardized stack traces for different use-cases may be added in the future.


