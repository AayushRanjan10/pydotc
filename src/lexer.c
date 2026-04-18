#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexer.h"

static const char* src;
static int pos = 0;

static int indent_stack[100] = {0}; 
static int stack_top = 0;

static Token peeked_token;
static int has_peeked = 0;

void init_lexer(const char* source_code) {
    src = source_code;
    pos = 0;

    stack_top = 0;
    indent_stack[0] = 0;

    has_peeked = 0;
}

// Needed for parser
Token peek_next_token() {
    if (!has_peeked) {
        peeked_token = get_next_token();
        has_peeked = 1;
    }
    return peeked_token;
}

Token get_next_token() {

    if (has_peeked) {
        has_peeked = 0;
        return peeked_token;
    }

    Token tok;

    while (src[pos] != '\0') {
        char c = src[pos];

        // If there is a space or tab in the middle of a line, we can ignore it
        if (c==' ' || c=='\t') {
            pos++;
            continue;
        }

        // Starting a new line; we count the spaces at the start of a newline to determine if we are stepping into a block (INDENT) or stepping out of a block (DEDENT).
        if (c=='\n') {
            pos++;
            int spaces = 0;
            while (src[pos] == ' ' || src[pos] == '\t') {
                if (src[pos] == ' ') {
                    spaces += 1;
                } else if (src[pos] == '\t') {
                    spaces += 4;
                }
                pos++;
            }
            if (spaces>indent_stack[stack_top]) {
                stack_top++;
                indent_stack[stack_top] = spaces;
                tok.type = INDENT;
                strcpy(tok.value, "INDENT");
                return tok;
            }
            else if (spaces<indent_stack[stack_top]) {
                indent_stack[stack_top] = 0;
                stack_top--;
                tok.type = DEDENT;
                strcpy(tok.value, "DEDENT");
                return tok;
            }
            else {
                tok.type = NEWLINE;
                strcpy(tok.value, "NEWLINE");
                return tok;
            }
        }

        // Single character tokens
        if (c=='+') {
            tok.type = PLUS;
            strcpy(tok.value, "+");
            pos++;
            return tok;
        }
        if (c=='-') {
            tok.type = MINUS;
            strcpy(tok.value, "-");
            pos++;
            return tok;
        }
        if (c=='(') {
            tok.type = LPAREN;
            strcpy(tok.value, "(");
            pos++;
            return tok;
        }
        if (c==')') {
            tok.type = RPAREN;
            strcpy(tok.value, ")");
            pos++;
            return tok;
        }
        if (c == '%') {
            tok.type = MODULUS;
            strcpy(tok.value, "%");
            pos++;
            return tok;
        }
        if (c == ':') {
            tok.type = COLON;
            strcpy(tok.value, ":");
            pos++;
            return tok;
        }

        // Characters which may be double-character tokens
        if (c=='*') {
            if (src[pos+1]=='*') {
                tok.type = POWER;
                strcpy(tok.value, "**");
                pos = pos+2;
                return tok;
            }
            else {
                tok.type = STAR;
                strcpy(tok.value, "*");
                pos++;
                return tok;
            }
        }
        else if (c=='/') {
            if (src[pos+1]=='/') {
                tok.type = FLOORDIV;
                strcpy(tok.value, "//");
                pos = pos+2;
                return tok;
            }
            else {
                tok.type = SLASH;
                strcpy(tok.value, "/");
                pos++;
                return tok;
            }
        }
        else if (c == '<') {
            if (src[pos+1] == '=') {
                tok.type = LTE;
                strcpy(tok.value, "<=");
                pos += 2;
                return tok;
            } 
            else {
                tok.type = LT;
                strcpy(tok.value, "<");
                pos++;
                return tok;
            }
        }
        else if (c == '>') {
            if (src[pos+1] == '=') {
                tok.type = GTE;
                strcpy(tok.value, ">=");
                pos += 2;
                return tok;
            } 
            else {
                tok.type = GT;
                strcpy(tok.value, ">");
                pos++;
                return tok;
            }
        }
        else if (c == '=') {
            if (src[pos+1] == '=') {
                tok.type = EQEQUAL; 
                strcpy(tok.value, "==");
                pos += 2;
                return tok;
            } 
            else {
                tok.type = ASSIGN;
                strcpy(tok.value, "=");
                pos++;
                return tok;
            }
        }
        else if (c == '!') {
            if (src[pos+1] == '=') {
                tok.type = NOTEQUAL; 
                strcpy(tok.value, "!=");
                pos += 2;
                return tok;
            } 
            else {
                // In Python, a '!' without an '=' is a syntax error
                tok.type = ERROR; 
                strcpy(tok.value, "!");
                pos++;
                return tok;
            }
        }

        // Numbers
        if (isdigit(src[pos])) {
            tok.type = NUMBER;
            char num[20];
            char *curr = num;
            while (isdigit(src[pos]) || src[pos]=='.') {
                *curr = src[pos];
                curr++;
                pos++;
            }
            *curr = '\0';
            strcpy(tok.value, num);
            return tok;
        }
        
        // Strings
        if (src[pos]=='"') {
            tok.type = STRING;
            char str[1000];
            char *curr = str;
            pos++;
            while (src[pos]!='"') {
                *curr = src[pos];
                curr++;
                pos++;  
            }
            pos++;
            *curr = '\0';
            strcpy(tok.value, str);
            return tok;
        }

        // Keywords and identifiers
        if (isalpha(src[pos]) || src[pos] == '_') {
            char id[1000];
            char *curr = id;
            while (isalnum(src[pos]) || src[pos] == '_') {
                *curr = src[pos];
                curr++;
                pos++;  
            }
            *curr = '\0';
            strcpy(tok.value, id);
            if (strcmp(tok.value, "print") == 0) tok.type = PRINT;
            else if (strcmp(tok.value, "while") == 0) tok.type = WHILE;
            else if (strcmp(tok.value, "if") == 0) tok.type = IF;
            else if (strcmp(tok.value, "elif") == 0) tok.type = ELIF;            
            else if (strcmp(tok.value, "else") == 0) tok.type = ELSE;
            else tok.type = IDENTIFIER;
            return tok;
        }

    }

    // End of File
    tok.type = END_OF_FILE;
    strcpy(tok.value, "EOF");
    return tok;
}