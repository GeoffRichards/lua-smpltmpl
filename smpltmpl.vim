" TODO: redefine luaError to link to Statement instead of Error, attempts so far have failed.

if exists("b:current_syntax")
    finish
endif

runtime! syntax/html.vim
unlet b:current_syntax

syn case match

syn cluster smpltmpl_top_cluster contains=smpltmpl_luacode

syn include @Lua $VIMRUNTIME/syntax/lua.vim
syn keyword luaFunc include

syn region  smpltmpl_luacode
            \ matchgroup=smpltmpl_tag
            \ start=+{!\=+
            \ end=+}+
            \ contains=@Lua keepend extend
syn region smpltmpl_luacode
            \ matchgroup=smpltmpl_tag
            \ start=+{{+
            \ end=+}}+
            \ contains=@Lua keepend extend

syn match smpltmpl_escape "\\\($\|[\\{}]\)"

syn sync minlines=50

hi def link smpltmpl_tag Type
hi def link smpltmpl_escape Special

syn cluster htmlPreProc add=@smpltmpl_top_cluster

let b:current_syntax = "smpltmpl"
