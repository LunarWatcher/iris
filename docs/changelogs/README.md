---
{
    "title": "Releases",
    "layout": "page_list",
    "type": "changelog"
}
---
# Releases

<ul>
{%- for it in listPages(".", violet.sort.by_creation_date) -%}
    {%- if it.layout != "page_list" -%}
        <li><a href="{{ it.url }}">{{ it.title }}</a>
{%- if exists("it.date") -%}
    ({{ formatDate(it.date, "%F") }})
{%- else -%}
    (unpublished)
{%- endif -%}
        </li>
    {%- endif -%}
{%- endfor -%}
</ul>
