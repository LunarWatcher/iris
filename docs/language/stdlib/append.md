---
{
    "title": "The append function"
}
---
# The append function

The `append` function is used to add an item to a list. 

## Signature

`append(list, newItem) -> null`

* `list`: The list to add to. Must be a `list`, or this function throws.
* `newItem`: The item to add to the list.

This function never returns a value. Changes are made in-place to the list. 

## Example

```iris
{% set list = [] %}
{{ list }} // prints []
{{ append(list, "item") }}
{{ list }} // prints ["item"]
```
