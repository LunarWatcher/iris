---
{
    "title": "Global macros"
}
---
# Global macros

Currently, there is no way to include macros in templates. This is largely because of lifetime issues and parsing issues, since it's assumed that a template is parsed once and then has the same-ish context. Macros break this, and as a result, includable macros has been postponed.

However, global macros are still an option, but require implementation on the API side. This is only valid as long as the same environment is reused for the macro definition and the render.

For a long list of reasons, macros are not made global by default. This list does include security reasons, but largely includes the fact that there's no guarantee that the parse order of files is preserved, so arbitrary files declaring global macros does not guarantee that those files have been parsed before any consumers of the macro are due to be parsed. This is an implementation detail that you may or may not have in your implementing application; this is not behaviour iris itself has a stake in.

You can, however, use `Environment::accept_macros(template)` to make macros available globally on a per-case basis:

```cpp
iris::Environment env;

auto tmpl = env.parse_file(
    "macros/macro_defs.iris"
);
env.accept_macros(tmpl);
```

## Suggested implementation

It's recommended that you have a dedicated folder or something to that effect that exclusively contains macros.

Violet uses `_macros/{filename}.iris`, where the filename itself is irrelevant, as each file may contain multiple macros.

## Suggested alternatives to macros

Iris already offers a C++ function API for custom functions. If you're using macros in place of C++ functions, consider if your use may be better served by C++ functions instead.

Macros should primarily be reserved for end-user use. If you're working with the API, you already have a more powerful option than macros available.

