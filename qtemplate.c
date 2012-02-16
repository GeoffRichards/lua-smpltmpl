#include "qtemplate.h"
#include <stdio.h>
#include <stdbool.h>

#define START_HTML \
    if (!inhtml) { \
        fprintf(fout, "    __out:write(\""); \
        inhtml = true; \
    }
#define END_HTML \
    if (inhtml) { \
        fprintf(fout, "\")\n"); \
        inhtml = false; \
    }

static int
compile_tmpl (lua_State *L) {
    const char *filename_in, *filename_out;
    FILE *fin, *fout;
    int c;
    int depth = 0;
    bool isexpr = false, inhtml = false, noescape = false;

    if (lua_gettop(L) != 2)
        return luaL_error(L, "wrong number of args to qtemplate.compile()");
    filename_in = lua_tostring(L, 1);
    filename_out = lua_tostring(L, 2);

    fin = fopen(filename_in, "rb");
    if (!fin) {
        fprintf(stderr, "can't open input file\n");
        return 1;
    }
    fout = fopen(filename_out, "wb");
    if (!fout) {
        fprintf(stderr, "can't open output file\n");
        return 1;
    }

    fprintf(fout, "-- Templated code compiled from %s\n"
            "local __M = {}\n\n"
            "local __env, __envmeta = {}, {}\n"
            "for k, v in pairs(getfenv()) do __env[k] = v end\n"
            "setmetatable(__env, __envmeta)\n\n"
            "function __M:generate (__out, __v)\n"
            "    __envmeta.__index = __v\n", filename_in);

    while ((c = fgetc(fin)) != EOF) {
        if (c == '{') {
            if (depth++ == 0) {
                END_HTML
                c = fgetc(fin);
                isexpr = true;
                noescape = false;
                if (c == '{')
                    isexpr = false;
                else if (c == '!')
                    noescape = true;
                else if (c != EOF)
                    ungetc(c, fin);
                if (isexpr)
                    fputs("    __out:write(tostring(", fout);
            }
            else
                fputc('{', fout);
        }
        else if (c == '}') {
            if (depth == 0) {
                fprintf(stderr, "unmatched '}'\n");
                return 1;
            }
            --depth;
            if (depth == 0) {
                if (isexpr) {
                    if (noescape)
                        fputs("))\n", fout);
                    else
                        fputs("):html())\n", fout);
                }
                else {
                    c = fgetc(fin);
                    if (c != '}') {
                        fprintf(stderr, "code chunk should end with '}}'\n");
                        return 1;
                    }
                    fputc('\n', fout);
                }
            }
            else
                fputc('}', fout);
        }
        else if (c == '$') {
            c = fgetc(fin);
            START_HTML
            if (c == EOF)
                fputc('$', fout);
            else
                fputc(c, fout);
        }
        else if (depth == 0) {
            START_HTML
            if (c == '\n')
                fputs("\\n", fout);
            else if (c == '\r')
                fputs("\\r", fout);
            else if (c == '\"')
                fputs("\\\"", fout);
            else
                fputc(c, fout);
        }
        else {
            fputc(c, fout);
        }
    }

    END_HTML
    if (depth != 0) {
        fprintf(stderr, "unclosed '{' at end of file\n");
        return 1;
    }

    fprintf(fout, "end\n\n"
            "setfenv(__M.generate, __env)\n\n"
            "return __M\n");

    fclose(fin);
    fclose(fout);
    return 0;
}

int
luaopen_qtemplate_priv (lua_State *L) {
#ifdef VALGRIND_LUA_MODULE_HACK
    /* Hack to allow Valgrind to access debugging info for the module. */
    luaL_getmetatable(L, "_LOADLIB");
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
#endif

    /* Create the table to return from 'require' */
    lua_createtable(L, 0, 3);
    lua_pushliteral(L, "_NAME");
    lua_pushliteral(L, "qtemplate_priv");
    lua_rawset(L, -3);
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, VERSION);
    lua_rawset(L, -3);
    lua_pushliteral(L, "compile");
    lua_pushcfunction(L, compile_tmpl);
    lua_rawset(L, -3);

    return 1;
}
