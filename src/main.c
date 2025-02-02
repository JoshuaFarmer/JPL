#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

char *src;
int tok;

void next();
void expr();
void emit_asm(char *fmt, ...);

enum {
        TOK_IF=0x100,
        TOK_ENDIF,
        TOK_FN,
        TOK_ENDFN,
        TOK_CAL,
        TOK_IDE,
};

FILE * fout=NULL;

void emit_asm(char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vfprintf(fout,fmt, args);
        va_end(args);
}

char ident[32];
void next() {
        while (*src == ' ' || *src == '\t' || *src == '\n') src++;
        if (isalpha(*src)) {
                int i = 0;
                while (isalpha(*src) && i < 31) {
                        ident[i++] = *src++;
                }
                ident[i] = '\0';
                if (strcmp(ident, "if") == 0) {
                        tok = TOK_IF;
                } else if (strcmp(ident, "call") == 0) {
                        tok = TOK_CAL;
                } else if (strcmp(ident, "endif") == 0) {
                        tok = TOK_ENDIF;
                } else if (strcmp(ident, "endsub") == 0) {
                        tok = TOK_ENDFN;
                } else if (strcmp(ident, "subroutine") == 0) {
                        tok = TOK_FN;
                } else if (i == 1) {
                        tok = ident[0];
                } else {
                        tok = TOK_IDE;
                }
        } else {
                tok = *src++;
        }
}

int get_i()
{
        int i = 0;
        while (isdigit(tok))
        {
                i *= 10;
                i += tok-'0';
                tok=*src++;
        }
        --src;
        return i;
}

int variable_addr(char name)
{
        if (name >= 'a' && name <= 'z')
        {
                return 0xFFFF - ((name-'a') * 4) - 4;
        }
}

char names[256];
unsigned char nameidx=0;

void pushName(char c)
{
        names[nameidx++]=c;
}

char popName()
{
        return names[--nameidx];
}

int start=1;

int           labels[256];
unsigned char labelpos=0;

void expr()
{
        static int label=0;
        if (tok == TOK_IF)
        {
                next();
                while (tok)
                {
                        expr();
                }

                labels[labelpos++]=label;
                emit_asm("\tcmp a,0\n");
                emit_asm("\tjz M%d\n",label++);
                start=1;
        }
        else if (tok == TOK_ENDIF)
        {
                int pos = labels[--labelpos];
                emit_asm("M%d\n",pos);
                next();
        }
        else if (tok == TOK_ENDFN)
        {
                emit_asm("\tret\n");
                next();
        }
        else if (tok == TOK_FN)
        {
                next();
                emit_asm("\thalt\n",ident);
                emit_asm("%s\n",ident);
                next();
        }
        else if (tok == TOK_CAL)
        {
                next();
                emit_asm("\tcall %s\n",ident);
                next();
        }
        else if (tok == ',')
        {
                start = 1;
                next();
        }
        else if (tok == '}')
        {
                tok = 0;
        }
        else if (tok >= 'a' && tok <= 'z')
        {
                pushName(tok);
                char nam = tok;
                next();
                if (start && tok != ':')
                {
                        popName();
                        emit_asm("\tmov a,[%d]\n",variable_addr(nam));
                        start = 0;
                }
                else if (!start)
                {
                        popName();
                        emit_asm("\tmov b,[%d]\n",variable_addr(nam));
                }
        }
        else if (isdigit(tok))
        {
                int i = get_i();
                if (start)
                {
                        emit_asm("\tmov a,%d\n", i);
                        start = 0;
                }
                else
                {
                        emit_asm("\tmov b,%d\n", i);
                }
                next();
        }
        else if (tok == '+')
        {
                next();
                expr();
                emit_asm("\tadd a,b\n", tok);
        }
        else if (tok == '-')
        {
                next();
                expr();
                emit_asm("\tsub a,b\n", tok);
        }
        else if (tok == ':')
        {
                char tok1=*src;
                next();
                next();
                while (tok && tok != ',')
                        expr();
                if (tok1 == '=')
                {
                        char name=popName();
                        emit_asm("\tmov [%d],a\n",variable_addr(name)); 
                }
                else if (tok1 == '+')
                {
                        char name=popName();
                        emit_asm("\tadd [%d],a\n",variable_addr(name)); 
                }
                else if (tok1 == '-')
                {
                        char name=popName();
                        emit_asm("\tsub [e],a\n",variable_addr(name)); 
                }
        }
        else if (tok == '=')
        {
                next();
                expr();
                emit_asm("\tcmp a,b\n"); 
                emit_asm("\tmzf\n"); 
        }
        else
        {
                printf("invalid: %d\n",tok);
                exit(1);
                tok=0;
        }
}

int main(int argc, char ** argv) {
        if (argc != 3) return 1;
        FILE * fp = fopen(argv[1],"r");
        fout = fopen(argv[2],"wb");
        if (!fp||!fout) return -2;
        char buff[513];
        src = buff;
        buff[512]=0;
        emit_asm("\t.org 0\n");
        emit_asm("start\n");
        while (fgets(buff,512,fp))
        {
                src=buff;
                next();
                start=1;
                while (tok)
                {
                        expr();
                }
        }
        emit_asm("\thalt\n");
        fclose(fp);
        return 0;
}