#include "smpltmpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t size, alloc;
    char *data;
} SimpleBuffer;

static void *
malloc_chked (size_t size) {
    void *data = malloc(size);
    if (!data) {
        fputs("smpltmpl: error allocating memory\n", stderr);
        exit(1);
    }
    return data;
}

static SimpleBuffer *
smplbuf_new (void) {
    SimpleBuffer *buf = malloc_chked(sizeof(SimpleBuffer));
    buf->size = 0;
    buf->alloc = 8192;
    buf->data = malloc_chked(buf->alloc);
    return buf;
}

static void
smplbuf_free (SimpleBuffer *buf) {
    free(buf->data);
    free(buf);
}

static void
smplbuf_pushlua (SimpleBuffer *buf, lua_State *L) {
    lua_pushlstring(L, buf->data, buf->size);
}

static void
smplbuf_ensure_available (SimpleBuffer *buf, size_t needed) {
    size_t total_needed = buf->size + needed;
    if (total_needed > buf->alloc) {
        size_t newsize = buf->alloc;
        char *newdata;
        while (newsize < total_needed)
            newsize *= 2;
        newdata = malloc_chked(newsize);
        memcpy(newdata, buf->data, buf->size);
        free(buf->data);
        buf->data = newdata;
        buf->alloc = newsize;
    }
}

static void
smplbuf_puts_len (SimpleBuffer *buf, const char *data, size_t size) {
    smplbuf_ensure_available(buf, size);
    memcpy(buf->data + buf->size, data, size);
    buf->size += size;
}

static void
smplbuf_putc (SimpleBuffer *buf, char c) {
    smplbuf_ensure_available(buf, 1);
    buf->data[buf->size++] = c;
}

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
static int
file_exists (lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    struct stat buf;
    lua_pushboolean(L, !stat(filename, &buf) || errno != ENOENT);
    return 1;
}

static int
file_mtime (lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    struct stat buf;
    if (stat(filename, &buf))   /* error */
        lua_pushnumber(L, (lua_Number) 0);
    else
        lua_pushnumber(L, (lua_Number) buf.st_mtime);
    return 1;
}

static int
syntax_err (lua_State *L, SimpleBuffer *buf, const char *filename,
            int line, int col, const char *err)
{
    smplbuf_free(buf);
    lua_pushfstring(L, "%s:%d:%d: %s", filename, line, col, err);
    return lua_error(L);
}

#define SYNTAX_ERR(err) syntax_err(L, buf, filename, line, col, (err))

#define SMPLBUF_PUTS(buf, data) \
    smplbuf_puts_len((buf), (data), strlen((data)))

#define START_HTML \
    if (!inhtml) { \
        SMPLBUF_PUTS(buf, "    __out:write(\""); \
        inhtml = 1; \
    }
#define END_HTML \
    if (inhtml) { \
        SMPLBUF_PUTS(buf, "\")\n"); \
        inhtml = 0; \
    }

static int
compile_tmpl (lua_State *L) {
    const char *data;
    size_t len, pos;
    int c;
    int depth = 0;
    int isexpr = 0, inhtml = 0;     /* these are bools */
    const char *filename;
    int line, col;
    SimpleBuffer *buf;

    if (lua_gettop(L) != 2)
        return luaL_error(L, "wrong number of args to smpltmpl.compile()");
    data = luaL_checklstring(L, 1, &len);
    filename = luaL_checkstring(L, 2);
    pos = 0;
    line = 1;
    col = 0;

    buf = smplbuf_new();
    SMPLBUF_PUTS(buf, "-- Template code compiled by Lua smpltmpl module.\n"
                 "local __M = {}\n\n"
                 "local __env, __envmeta = {}, {}\n"
                 "for k, v in pairs(_ENV) do __env[k] = v end\n"
                 "setmetatable(__env, __envmeta)\n\n"
                 "function __M:generate (__self, Tmpl, __out, __v)\n"
                 "    __envmeta.__index = __v\n"
                 "    local _ENV = __env\n"
                 "    local function include (name, vars)\n"
                 "        __self:_include(__out, name, (vars or __env))\n"
                 "    end\n");

    while (pos < len) {
        c = data[pos++];
        col++;
        if (c == '\n') {
            line++;
            col = 1;
        }

        if (depth != 0) {
            int skipped = 0;        /* bool */
            size_t start_pos = pos - 1;

            if (c == '-' && data[pos] == '-') {     /* skip comments */
                skipped = 1;
                while (data[pos++] != '\n')
                    ;
                line++;
            }
            else if (c == '"' || c== '\'') {        /* skip string literals */
                char quote = c;
                skipped = 1;
                do {
                    c = data[pos++];
                    col++;
                    if (c == '\\') {
                        if (data[pos++] == '\n') {
                            line++;
                            col = 1;
                        }
                        else
                            col++;
                    }
                } while (c != quote);
            }

            if (skipped) {
                smplbuf_puts_len(buf, data + start_pos, pos - start_pos);
                continue;
            }
        }

        if (c == '{') {
            if (depth++ == 0) {
                int noescape = 0;   /* bool */
                END_HTML
                isexpr = 1;
                if (pos < len) {
                    c = data[pos++];
                    col++;
                    if (c == '{')
                        isexpr = 0;
                    else if (c == '!')
                        noescape = 1;
                    else {
                        pos--;
                        col--;
                    }
                }
                if (isexpr) {
                    if (noescape)
                        SMPLBUF_PUTS(buf, "    __out:write((tostring(");
                    else
                        SMPLBUF_PUTS(buf, "    __out:write(Tmpl.escape_html(tostring(");
                }
            }
            else
                smplbuf_putc(buf, '{');
        }
        else if (c == '}') {
            if (depth == 0)
                return SYNTAX_ERR("unmatched '}'");
            --depth;
            if (depth == 0) {
                if (isexpr)
                    SMPLBUF_PUTS(buf, ")))\n");
                else {
                    if (pos >= len || data[pos++] != '}')
                        return SYNTAX_ERR("code chunk should end with '}}'");
                    col++;
                    smplbuf_putc(buf, '\n');
                }
            }
            else
                smplbuf_putc(buf, '}');
        }
        else if (c == '\\') {
            if (pos >= len)
                return SYNTAX_ERR("can't have '\\' at end of file");
            c = data[pos++];
            col++;
            if (c == '\\' || c == '{' || c == '}') {
                col++;
                START_HTML
                smplbuf_putc(buf, c);
                if (c == '\\')
                    smplbuf_putc(buf, c);       /* escape in Lua source */
            }
            else if (c == '\n') {   /* skip LF */
                line++;
                col = 1;
            }
            else if (c == '\r') {   /* skip CR, and LF if there is one */
                line++;
                col = 1;
                if (pos < len && data[pos] == '\n')
                    pos++;
            }
            else
                return SYNTAX_ERR("unexpected character after '\\'");
        }
        else if (depth == 0) {
            START_HTML
            if (c == '\n')
                SMPLBUF_PUTS(buf, "\\n");
            else if (c == '\r')
                SMPLBUF_PUTS(buf, "\\r");
            else if (c == '\"')
                SMPLBUF_PUTS(buf, "\\\"");
            else
                smplbuf_putc(buf, c);
        }
        else {
            smplbuf_putc(buf, c);
        }
    }

    END_HTML
    if (depth != 0)
        return SYNTAX_ERR("unclosed '{' at end of file");

    SMPLBUF_PUTS(buf, "end\n\n"
                 "return __M\n");
    smplbuf_pushlua(buf, L);
    smplbuf_free(buf);
    return 1;
}

int
luaopen_smpltmpl_priv (lua_State *L) {
#ifdef VALGRIND_LUA_MODULE_HACK
    /* Hack to allow Valgrind to access debugging info for the module. */
    luaL_getmetatable(L, "_LOADLIB");
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
#endif

    /* Create the table to return from 'require' */
    lua_createtable(L, 0, 5);
    lua_pushliteral(L, "_NAME");
    lua_pushliteral(L, "smpltmpl_priv");
    lua_rawset(L, -3);
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, VERSION);
    lua_rawset(L, -3);
    lua_pushliteral(L, "compile");
    lua_pushcfunction(L, compile_tmpl);
    lua_rawset(L, -3);
    lua_pushliteral(L, "_file_exists");
    lua_pushcfunction(L, file_exists);
    lua_rawset(L, -3);
    lua_pushliteral(L, "_file_mtime");
    lua_pushcfunction(L, file_mtime);
    lua_rawset(L, -3);

    return 1;
}
