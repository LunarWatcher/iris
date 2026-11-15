---
{
    "title": "User-defined functions (macros)"
}
---
# User-defined functions (macros)

Iris supports `{% macro %}` statements. These statements can be used to define custom functions.

## Syntax

```iris
{% macro functionName(arg1, arg2, arg3 = "defaulted", arg4 = 69) %}
The contents of this {{ arg1 }} can be reused in multiple {{ arg2 }}. Arguments may be {{ arg3 }}, and may be non-strings: {{ arg4 }} + 4 = {{ arg4 + 4 }}.

Macros support all the same syntax as the rest of iris, with the exception of further macro statements (see the limitations section).
{% endmacro %}

Using a macro is identical to any other function call. Defaulted arguments are optional:
{{ functionName("macro", "places") }}

Defaulted arguments may be defined anyway.
{{ functionName("macro", "places", "defaulted or overridden", 65) }}

Not all default args have to be specified:
{{ functionName("macro", "places", "defaulted or overridden") }}
```

## Limitations

### Macro nesting is forbidden

Macros may not be nested, meaning you cannot declare a macro within another macro. You can call macros recursively, though doing so is probably a bad idea.

### Macros may not be set globally unless explicitly allowed

Macros may be set globally, though this requires the implementing software to support it. By default, macros are local to the file.

### Macros may not override or shadow builtin functions, or non-builtins defined in C++

Macros may override or shadow other macros, but builtin functions and non-builtin functions defined by the implementing software always take precedence. Prefixing your macros to avoid conflict with builtin functions and non-builtins from the implementing software is recommended.

**DO:** `{% macro my_add_numbers() %}`

**DON'T:** `{% macro add_numbers() %}`

Note that the prefix itself is not important, and the exact prefixes to use or to avoid vary by the implementing software.
