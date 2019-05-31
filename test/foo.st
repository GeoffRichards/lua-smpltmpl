[{ hello }] [{!Tmpl.escape_html(hello)}] [{! hello }]

Escaped: \\ \{ \} <x>&amp;</x> ' "

tostring: { 23} {! 42}
floats: { 23.0000} {! 42.0000}

Nested braces: [{"x"} { #{ {{ {}}} , {{}} } }{{
    -- foo {} {{}} }}}}
    x = "bar \"\''\
 {} {{}} }}}} \\"
    x = 'bar \"\'"\
 {} {{}} }}}} \\'
    x = "foo"
}} {x}]

[\
{{ for _, item in ipairs(list) do }}\
* { item }.
{{ end }}\
]

{{
include("inc", { incvar = "first" })
incvar = "second"
include("inc")
}}
