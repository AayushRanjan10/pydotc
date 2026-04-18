#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_BINOP,
    AST_ASSIGN,
    AST_PRINT,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    struct ASTNode* next; // We are storing the statement nodes in a linked list since vectors are not there in C
    // We are using union to optimize memory, since different types of nodes require us to store different types of variables
    union {
        // If type == AST_NUMBER, we use this:
        double number_value; 
        
        // If type == AST_IDENTIFIER, we use this:
        char variable_name[100]; 
        
        // If type == AST_BINOP, we use this:
        struct {
            struct ASTNode* left;
            TokenType operator_type;
            struct ASTNode* right;
        } binop;

        // If type == AST_ASSIGN
        struct {
            char variable_name[100];
            struct ASTNode* value; 
        } assign;

        // If type == AST_PRINT
        struct {
            struct ASTNode* expression; 
        } print_stmt;

        // If type == AST_PROGRAM
        struct {
            struct ASTNode* head_statement;
        } program;
    } data;

} ASTNode;

void init_parser(const char* source_code);
ASTNode* parse_program();
#endif