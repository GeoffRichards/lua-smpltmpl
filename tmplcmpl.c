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

int
main (int argc, const char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: tmplcmpl input.tmpl output.lua\n");
        return 1;
    }
    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        fprintf(stderr, "can't open input file\n");
        return 1;
    }
    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        fprintf(stderr, "can't open output file\n");
        return 1;
    }

    fprintf(fout, "-- Templated code compiled from %s\n"
            "local __M = {}\n\n"
            "local __env, __envmeta = {}, {}\n"
            "for k, v in pairs(getfenv()) do __env[k] = v end\n"
            "setmetatable(__env, __envmeta)\n\n"
            "function __M.generate (__out, __v)\n"
            "    __envmeta.__index = __v\n", argv[1]);

    int c;
    int depth = 0;
    bool isexpr = false, inhtml = false, noescape = false;
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
