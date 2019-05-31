This Lua library provides a simple text/HTML templating system.

See the documentation in the _doc/_ directory (in POD and man(1)
format) for details.  The documentation is also available online
at the module's website:

<https://geoffrichards.co.uk/lua/smpltmpl/>

This module includes C code that needs to be compiled.  For Linux
systems the Makefile provided should do the trick.


## Syntax highlighting

There's a Vim syntax highlighting file in _smpltmpl.vim_, although
it has problems with the current version of Lua syntax highlighting
when you have Lua keywords like `end` seen out of context in parts
of a template. I haven't been able to figure out how to fix that,
so the best I can recommend is changing the installed _lua.vim_ file,
changing the entry that sets the meaning of the `luaError` syntax
from `Error` to `Statement`. That results in a line like:

    HiLink luaError Statement
