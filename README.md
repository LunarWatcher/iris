# Iris templates

Iris is a template engine for modern C++, loosely inspired by [jinja](http://jinja.pocoo.org) for python. It has an easy and yet powerful template syntax with all variables, loops, conditions, includes, callbacks, and comments you need, nested and combined as you like. Here is what it looks like:

```cpp
iris::DataObject obj = iris::Object {
    {"name", "world"},
};

iris::render("Hello {{ name }}!", data); // Returns "Hello world!"
```

Iris is a fork of [inja](https://github.com/pantor/inja/) with stronger guarantees against AI slop, as well as changes meant to benefit [violet](https://codeberg.org/LunarWatcher/violet) and whatever else in the general anti-AI ecosystem could need proper templates beyond what something more simple like mustache can provide.

## Tutorial

This tutorial will give you an idea how to use iris. It will explain the most important concepts and give practical advices using examples and executable code. Beside this tutorial, you may check out the [documentation](https://lunarwatcher.codeberg.page/iris).

### Template Rendering

The basic template rendering takes a template as a `std::string` and an `iris::DataObject` for all data. It returns the rendered template as an `std::string`.

```cpp
iris::DataObject obj = iris::Object {
    {"name", "world"},
};

render("Hello {{ name }}!", data); // Returns std::string "Hello world!"
render_to(std::cout, "Hello {{ name }}!", data); // Writes "Hello world!" to stream
```

For more advanced usage, an environment is recommended.
```cpp
Environment env;

// Render a string with data
std::string result = env.render("Hello {{ name }}!", data); // "Hello world!"

// Or directly read a template file
Template temp = env.parse_template("./templates/greeting.txt");
std::string result = env.render(temp, data); // "Hello world!"

data["name"] = "Iris";
std::string result = env.render(temp, data); // "Hello Iris!"

// Or read the template file directly from the environment
result = env.render_file("./templates/greeting.txt", data);

// Or write a rendered template file
env.write(temp, data, "./result.txt");
```

The environment class can be configured to your needs.
```cpp
// With default settings
Environment env_default;

// With global path to template files and where files will be saved
Environment env_1 {"../path/templates/"};

// With separate input and output path
Environment env_2 {"../path/templates/", "../path/results/"};

// With other opening and closing strings (here the defaults)
env.set_expression("{{", "}}"); // Expressions
env.set_comment("{#", "#}"); // Comments
env.set_statement("{%", "%}"); // Statements {% %} for many things, see below
env.set_line_statement("##"); // Line statements ## (just an opener)
env.set_html_autoescape(true); // Perform HTML escaping on all strings
```

### Variables

Variables are rendered within the `{{ ... }}` expressions.
```cpp
iris::DataObject obj = iris::Object {};
data["neighbour"] = "Peter";
data["guests"] = {"Jeff", "Tom", "Patrick"};
data["time"]["start"] = 16;
data["time"]["end"] = 22;

// Indexing in array
render("{{ guests.1 }}", data); // "Tom"

// Objects
render("{{ time.start }} to {{ time.end + 1 }}pm", data); // "16 to 23pm"
```
If no variable is found, valid JSON is printed directly, otherwise an `iris::RenderError` is thrown.

### Statements

Statements can be written either with the `{% ... %}` syntax or the `##` syntax for entire lines. Note that `##` needs to start the line without indentation. The most important statements are loops, conditions and file includes. All statements can be nested.

#### Loops

```cpp
// Combining loops and line statements
render(R"(Guest List:
## for guest in guests
	{{ loop.index1 }}: {{ guest }}
## endfor )", data)

/* Guest List:
	1: Jeff
	2: Tom
	3: Patrick */
```
In a loop, the special variables `loop.index (number)`, `loop.index1 (number)`, `loop.is_first (boolean)` and `loop.is_last (boolean)` are defined. In nested loops, the parent loop variables are available e.g. via `loop.parent.index`. You can also iterate over objects like `{% for key, value in time %}`.

#### Conditions

Conditions support the typical if, else if and else statements. Following conditions are for example possible:
```cpp
// Standard comparisons with a variable
render("{% if time.hour >= 20 %}Serve{% else if time.hour >= 18 %}Make{% endif %} dinner.", data); // Serve dinner.

// Variable in list
render("{% if neighbour in guests %}Turn up the music!{% endif %}", data); // Turn up the music!

// Logical operations
render("{% if guest_count < (3+2) and all_tired %}Sleepy...{% else %}Keep going...{% endif %}", data); // Sleepy...

// Negations
render("{% if not guest_count %}The End{% endif %}", data); // The End
```

#### Includes

You can either include other in-memory templates or from the file system.
```cpp
// To include in-memory templates, add them to the environment first
iris::Template content_template = env.parse("Hello {{ neighbour }}!");
env.include_template("content", content_template);
env.render("Content: {% include \"content\" %}", data); // "Content: Hello Peter!"

// Other template files are included relative from the current file location
render("{% include \"footer.html\" %}", data);
```
If a corresponding template could not be found in the file system, the *include callback* is called:
```cpp
// The callback takes the current path and the wanted include name and returns a template
env.set_include_callback([&env](const std::filesystem::path& path, const std::string& template_name) {
  return env.parse("Hello {{ neighbour }} from " + template_name);
});

// You can disable to search for templates in the file system via
env.set_search_included_templates_in_files(false);
```

Iris will throw an `iris::RenderError` if an included file is not found and no callback is specified. To disable this error, you can call `env.set_throw_at_missing_includes(false)`.

#### Assignments

Variables can also be defined within the template using the set statment.
```cpp
render("{% set new_hour=23 %}{{ new_hour }}pm", data); // "23pm"
render("{% set time.start=18 %}{{ time.start }}pm", data); // using json pointers
```

Assignments only set the value within the rendering context; they do not modify the json object passed into the `render` call.

### Functions

A few functions are implemented within the iris template syntax. They can be called with
```cpp
// Upper, lower and capitalize function, for string cases
render("Hello {{ upper(neighbour) }}!", data); // "Hello PETER!"
render("Hello {{ lower(neighbour) }}!", data); // "Hello peter!"
render("Hello {{ capitalize(neighbour) }}!", data); // "Hello Peter!"

// Replace characters in a string
render("{{ replace(neighbour, \"e\", \"3\")}}", data); // "P3t3r"

// Range function, useful for loops
render("{% for i in range(4) %}{{ loop.index1 }}{% endfor %}", data); // "1234"
render("{% for i in range(3) %}{{ at(guests, i) }} {% endfor %}", data); // "Jeff Tom Patrick "

// Length function (please don't combine with range, use list directly...)
render("I count {{ length(guests) }} guests.", data); // "I count 3 guests."

// Get first and last element in a list
render("{{ first(guests) }} was first.", data); // "Jeff was first."
render("{{ last(guests) }} was last.", data); // "Patir was last."

// Sort a list
render("{{ sort([3,2,1]) }}", data); // "[1,2,3]"
render("{{ sort(guests) }}", data); // "[\"Jeff\", \"Patrick\", \"Tom\"]"

// Join a list with a separator
render("{{ join([1,2,3], \" + \") }}", data); // "1 + 2 + 3"
render("{{ join(guests, \", \") }}", data); // "Jeff, Patrick, Tom"

// Round numbers to a given precision
render("{{ round(3.1415, 0) }}", data); // 3
render("{{ round(3.1415, 3) }}", data); // 3.142

// Check if a value is odd, even or divisible by a number
render("{{ odd(42) }}", data); // false
render("{{ even(42) }}", data); // true
render("{{ divisibleBy(42, 7) }}", data); // true

// Maximum and minimum values from a list
render("{{ max([1, 2, 3]) }}", data); // 3
render("{{ min([-2.4, -1.2, 4.5]) }}", data); // -2.4

// Convert strings to numbers
render("{{ int(\"2\") == 2 }}", data); // true
render("{{ float(\"1.8\") > 2 }}", data); // false

// Set default values if variables are not defined
render("Hello {{ default(neighbour, \"my friend\") }}!", data); // "Hello Peter!"
render("Hello {{ default(colleague, \"my friend\") }}!", data); // "Hello my friend!"

// Access an objects value dynamically
render("{{ at(time, \"start\") }} to {{ time.end }}", data); // "16 to 22"

// Check if a key exists in an object
render("{{ exists(\"guests\") }}", data); // "true"
render("{{ exists(\"city\") }}", data); // "false"
render("{{ existsIn(time, \"start\") }}", data); // "true"
render("{{ existsIn(time, neighbour) }}", data); // "false"

// Check if a key is a specific type
render("{{ isString(neighbour) }}", data); // "true"
render("{{ isArray(guests) }}", data); // "true"
// Implemented type checks: isArray, isBoolean, isFloat, isInteger, isNumber, isObject, isString,
```

The Jinja2 pipe call syntax of functions is also supported:

```cpp
// Upper neighbour value
render("Hello {{ neighbour | upper }}!", data); // "Hello PETER!"

// Sort array and join with comma
render("{{ [\"B\", \"A\", \"C\"] | sort | join(\",\") }}", data); // "A,B,C"
```

### Template Inheritance

Template inheritance allows you to build a base *skeleton* template that contains all the common elements and defines blocks that child templates can override. Lets show an example: The base template
```.html
<!DOCTYPE html>
<html>
<head>
  {% block head %}
  <link rel="stylesheet" href="style.css" />
  <title>{% block title %}{% endblock %} - My Webpage</title>
  {% endblock %}
</head>
<body>
  <div id="content">{% block content %}{% endblock %}</div>
</body>
</html>
```
contains three `blocks` that child templates can fill in. The child template
```.html
{% extends "base.html" %}
{% block title %}Index{% endblock %}
{% block head %}
  {{ super() }}
  <style type="text/css">
    .important { color: #336699; }
  </style>
{% endblock %}
{% block content %}
  <h1>Index</h1>
  <p class="important">
    Welcome to my blog!
  </p>
{% endblock %}
```
calls a parent template with the `extends` keyword; it should be the first element in the template. It is possible to render the contents of the parent block by calling `super()`. In the case of multiple levels of `{% extends %}`, super references may be called with an argument (e.g. `super(2)`) to skip levels in the inheritance tree.

### Whitespace Control

In the default configuration, no whitespace is removed while rendering the file. To support a more readable template style, you can configure the environment to control whitespaces before and after a statement automatically. While enabling `set_trim_blocks` removes the first newline after a statement, `set_lstrip_blocks` strips tabs and spaces from the beginning of a line to the start of a block.

```cpp
Environment env;
env.set_trim_blocks(true);
env.set_lstrip_blocks(true);
```

With both `trim_blocks` and `lstrip_blocks` enabled, you can put statements on their own lines. Furthermore, you can also strip whitespaces for both statements and expressions by hand. If you add a minus sign (`-`) to the start or end, the whitespaces before or after that block will be removed:

```cpp
render("Hello       {{- name -}}     !", data); // "Hello Iris!"
render("{% if neighbour in guests -%}   I was there{% endif -%}   !", data); // Renders without any whitespaces
```

Stripping behind a statement or expression also removes any newlines.

### HTML escaping

Templates are frequently used to creat HTML pages. Source data that contains
characters that have meaning within HTML (like <. >, &) needs to be escaped.
It is often inconvenient to perform such escaping within the data. With `Environment::set_html_autoescape(true)`, Iris can be configured to
HTML escape each and every string created.

### Comments

Comments can be written with the `{# ... #}` syntax.
```cpp
render("Hello{# Todo #}!", data); // "Hello!"
```

### Exceptions

Iris uses exceptions to handle ill-formed template input. However, exceptions can be switched off with either using the compiler flag `-fno-exceptions` or by defining the symbol `IRIS_NOEXCEPTION`. In this case, exceptions are replaced by `abort()` calls.


## Supported compilers

Iris requires C++23. 
