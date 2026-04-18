#ifndef LEXER_H
#define LEXER_H

typedef enum {
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MODULUS,
    POWER,
    FLOORDIV,
    ASSIGN,
    LPAREN,
    RPAREN,
    LT,
    GT,
    LTE,
    GTE,
    EQEQUAL,
    NOTEQUAL,
    COLON,

    PRINT,
    WHILE,
    IF,
    ELIF,
    ELSE,

    NUMBER,
    IDENTIFIER,
    STRING,

    NEWLINE,
    INDENT,
    DEDENT,

    ERROR,

    END_OF_FILE
} TokenType;


typedef struct {
    TokenType type;
    char value[1000];
} Token;

void init_lexer(const char* source_code);
Token get_next_token();
Token peek_next_token(); // Needed for parser logic, to check next token without consuming it

#endif