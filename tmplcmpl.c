#include <stdio.h>
#include <stdbool.h>

#define START_HTML \
    if (!inhtml) { \
        fprintf(fout, "    res:write(\""); \
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
            "local M = {}\n"
            "function M.generate (res, v)\n", argv[1]);

    int c;
    int depth = 0;
    bool isexpr = false, inhtml = false, noescape = false;
    while ((c = fgetc(fin)) != EOF) {
        if (c == '{') {
            if (depth++ == 0) {
                END_HTML
                c = fgetc(fin);
                isexpr = false;
                noescape = false;
                if (c == '=') {
                    isexpr = true;
                    c = fgetc(fin);
                    if (c == '!')
                        noescape = true;
                    else if (c != EOF)
                        ungetc(c, fin);
                }
                else if (c != EOF)
                    ungetc(c, fin);
                if (isexpr)
                    fputs("    res:write((", fout);
            }
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
                else
                    fputc('\n', fout);
            }
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

    fprintf(fout, "end\n"
            "return M\n");

    fclose(fin);
    fclose(fout);
    return 0;
}
