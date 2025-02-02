#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <libgen.h>

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
        TOK_WHILE,
        TOK_ENDW,
        TOK_RET,
};

FILE * fout=NULL;

void emit_asm(char *fmt, ...) {
        char * f = strdup(fmt);
        va_list args;
        va_start(args, fmt);
        char * x = strtok(f,"\\");
        int start=1;
        while (x != NULL)
        {
                if (!start)
                        fprintf(fout,"\n");
                vfprintf(fout,x,args);
                x = strtok(NULL,"\\\0");
                start=0;
        }
        va_end(args);
        free(f); f = NULL;
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
                } else if (strcmp(ident, "while") == 0) {
                        tok = TOK_WHILE;
                } else if (strcmp(ident, "endw") == 0) {
                        tok = TOK_ENDW;
                } else if (strcmp(ident, "endif") == 0) {
                        tok = TOK_ENDIF;
                } else if (strcmp(ident, "endsub") == 0) {
                        tok = TOK_ENDFN;
                } else if (strcmp(ident, "return") == 0) {
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
                return ((name-'a') * 4);
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

char * TOK_TO_ASM[18];

enum
{
        TO_EMIT_ADD,
        TO_EMIT_SUB,
        TO_EMIT_IF,
        TO_EMIT_ENDIF,
        TO_EMIT_SUBR,
        TO_EMIT_ENDSUB,
        TO_EMIT_CALL,
        TO_EMIT_VAR_GET_1,
        TO_EMIT_VAR_GET_2,
        TO_EMIT_NUM_1,
        TO_EMIT_NUM_2,
        TO_EMIT_SET,
        TO_EMIT_SET_ADD,
        TO_EMIT_SET_SUB,
        TO_EMIT_EQUAL,
        TO_EMIT_STOP,
        TO_EMIT_WHILE,
        TO_EMIT_ENDWHILE,
};

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
                emit_asm(TOK_TO_ASM[TO_EMIT_IF],label++);
                start=1;
        }
        else if (tok == TOK_WHILE)
        {
                labels[labelpos++]=label;
                emit_asm("M%d:\n",label++);
                next();
                while (tok)
                {
                        expr();
                }

                labels[labelpos++]=label;
                emit_asm(TOK_TO_ASM[TO_EMIT_WHILE],label++);
                start=1;
        }
        else if (tok == TOK_ENDIF)
        {
                int pos = labels[--labelpos];
                emit_asm(TOK_TO_ASM[TO_EMIT_ENDIF],pos);
                next();
        }
        else if (tok == TOK_ENDW)
        {
                int pos1 = labels[--labelpos];
                int pos2 = labels[--labelpos];
                emit_asm(TOK_TO_ASM[TO_EMIT_ENDWHILE],pos2,pos1);
                next();
        }
        else if (tok == TOK_ENDFN)
        {
                emit_asm(TOK_TO_ASM[TO_EMIT_ENDSUB]);
                next();
        }
        else if (tok == TOK_FN)
        {
                next();
                emit_asm(TOK_TO_ASM[TO_EMIT_SUBR],ident);
                next();
        }
        else if (tok == TOK_CAL)
        {
                next();
                emit_asm(TOK_TO_ASM[TO_EMIT_CALL],ident);
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
                        emit_asm(TOK_TO_ASM[TO_EMIT_VAR_GET_1],variable_addr(nam));
                        start = 0;
                }
                else if (!start)
                {
                        popName();
                        emit_asm(TOK_TO_ASM[TO_EMIT_VAR_GET_2],variable_addr(nam));
                }
        }
        else if (isdigit(tok))
        {
                int i = get_i();
                if (start)
                {
                        emit_asm(TOK_TO_ASM[TO_EMIT_NUM_1], i);
                        start = 0;
                }
                else
                {
                        emit_asm(TOK_TO_ASM[TO_EMIT_NUM_2], i);
                }
                next();
        }
        else if (tok == '+')
        {
                next();
                expr();
                emit_asm(TOK_TO_ASM[TO_EMIT_ADD]);
        }
        else if (tok == '-')
        {
                next();
                expr();
                emit_asm(TOK_TO_ASM[TO_EMIT_SUB]);
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
                        emit_asm(TOK_TO_ASM[TO_EMIT_SET],variable_addr(name)); 
                }
                else if (tok1 == '+')
                {
                        char name=popName();
                        emit_asm(TOK_TO_ASM[TO_EMIT_SET_ADD],variable_addr(name)); 
                }
                else if (tok1 == '-')
                {
                        char name=popName();
                        emit_asm(TOK_TO_ASM[TO_EMIT_SET_SUB],variable_addr(name)); 
                }
        }
        else if (tok == '=')
        {
                next();
                expr();
                emit_asm(TOK_TO_ASM[TO_EMIT_EQUAL]); 
        }
        else
        {
                printf("invalid: %d",tok);
                exit(1);
                tok=0;
        }
}

void getArch(const char *self_location, const char *path) {
        char self_copy[256];
        strncpy(self_copy, self_location, sizeof(self_copy) - 1);
        self_copy[sizeof(self_copy) - 1] = '\0';
        char *dir = dirname(self_copy);
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s.ad", dir, path);
        FILE * fp = fopen(full_path, "r");
        if (!fp)
        {
                printf("Arch not found!\n");
                exit(1);
        }
        char buff[513];
        while (fgets(buff,512,fp))
        {
                char * PROP = strtok(buff,"@");
                char * TEXT = strtok(NULL,"\0");
                int idx=-1;
                if (strcmp(PROP,"TOK_ADD")==0) { idx=TO_EMIT_ADD; }
                else if (strcmp(PROP,"TOK_SUB")==0) { idx=TO_EMIT_SUB; }
                else if (strcmp(PROP,"TOK_IF")==0) { idx=TO_EMIT_IF; }
                else if (strcmp(PROP,"TOK_ENDIF")==0) { idx=TO_EMIT_ENDIF; }
                else if (strcmp(PROP,"TOK_ENDSUB")==0) { idx=TO_EMIT_ENDSUB; }
                else if (strcmp(PROP,"TOK_SUBR")==0) { idx=TO_EMIT_SUBR; }
                else if (strcmp(PROP,"TOK_CALL")==0) { idx=TO_EMIT_CALL; }
                else if (strcmp(PROP,"TOK_VAR_GET_1")==0) { idx=TO_EMIT_VAR_GET_1; }
                else if (strcmp(PROP,"TOK_VAR_GET_2")==0) { idx=TO_EMIT_VAR_GET_2; }
                else if (strcmp(PROP,"TOK_NUM_1")==0) { idx=TO_EMIT_NUM_1; }
                else if (strcmp(PROP,"TOK_NUM_2")==0) { idx=TO_EMIT_NUM_2; }
                else if (strcmp(PROP,"TOK_SET")==0) { idx=TO_EMIT_SET; }
                else if (strcmp(PROP,"TOK_ADD_SET")==0) { idx=TO_EMIT_SET_ADD; }
                else if (strcmp(PROP,"TOK_SUB_SET")==0) { idx=TO_EMIT_SET_SUB; }
                else if (strcmp(PROP,"TOK_ISEQUAL")==0) { idx=TO_EMIT_EQUAL; }
                else if (strcmp(PROP,"TOK_WHILE")==0) { idx=TO_EMIT_WHILE; }
                else if (strcmp(PROP,"TOK_ENDWHILE")==0) { idx=TO_EMIT_ENDWHILE; }
                else if (strcmp(PROP,"HLT")==0) { idx=TO_EMIT_STOP; }
                else
                {
                        printf("Invalid Option: %s\n",PROP);
                        exit(1);
                }
                TOK_TO_ASM[idx] = strdup(TEXT);
        }
        fclose(fp); fp = NULL;
        printf("Found Arch\n");
}

void compile(const char * path)
{
        FILE * fp = fopen(path,"r");
        if (!fp||!fout) {
                printf("File does not exist: %s\n",path);
                return;
        }
        char buff[513];
        buff[512]=0;
        while (fgets(buff,512,fp))
        {
                if (strncmp(buff,"include",7)==0)
                {
                        char * x = strtok(buff," ");
                        char * include = strtok(NULL,"\n\r\0");
                        compile(include);
                        continue;
                }
                src=buff;
                next();
                start=1;
                while (tok)
                {
                        expr();
                }
        }
        fclose(fp);
}

int main(int argc, char ** argv) {
        if (argc >= 4)
        {
                getArch(argv[0], argv[3]);
                fout = fopen(argv[2],"wb");
                emit_asm("start:\n");
                emit_asm("  call Smain\n");
                if (argc == 5)
                {
                        FILE * fp = fopen(argv[4],"rb");
                        if (!fp) return -2;
                        char buff[8192];
                        fread(buff,1,8192,fp);
                        emit_asm(buff);
                        fclose(fp); fp = NULL;
                }
                compile(argv[1]);
                emit_asm(TOK_TO_ASM[TO_EMIT_STOP]);
                for (int i = 0; i < 18; ++i)
                {
                        if (TOK_TO_ASM[i])
                        {
                        free(TOK_TO_ASM[i]);
                        }
                }
                emit_asm("\nvars:\n");
                for (int i = 0; i < 26; ++i)
                {
                        emit_asm("  dd 0\n",65+i);
                }
                fclose(fout); fout = NULL;
                return 0;
        }
}