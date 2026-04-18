#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static Token current_token;

// Grabs the next token from the lexer
static void advance() {
    current_token = get_next_token();
}

// Validates the current token and advances, or crashes if grammar is broken
static void eat(TokenType expected_type) {
    if (current_token.type == expected_type) {
        advance();
    }
    else {
        printf("Syntax Error");
        exit(1); 
    }
}

// Allocates memory for a new AST Node on the heap
static ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    return node;
}

// We must declare these here because they constantly call each other
static ASTNode* parse_statement();
static ASTNode* parse_expression();
static ASTNode* parse_term();
static ASTNode* parse_power();
static ASTNode* parse_factor();


static ASTNode* parse_factor() {
    Token tok = current_token;
    if (tok.type==NUMBER) {
        eat(NUMBER);
        ASTNode* node = create_node(AST_NUMBER);
        node->data.number_value = strtod(tok.value, NULL); 
        return node;
    }
    else if (tok.type == IDENTIFIER) {
        eat(IDENTIFIER);
        ASTNode* node = create_node(AST_IDENTIFIER);
        strcpy(node->data.variable_name, tok.value);
        return node;
    }
    else if (tok.type == LPAREN) {
        eat(LPAREN);
        // If there is an open paranthesis, we must parse the expression.
        ASTNode* node = parse_expression(); 
        eat(RPAREN);
        return node;
    }
    else {
        printf("Syntax Error");
        exit(1); 
    }
}

static ASTNode* parse_power() {
    ASTNode* left_node = parse_factor();
    Token tok = current_token;
    if (tok.type == POWER) {
        eat(POWER);
        ASTNode *new_node = create_node(AST_BINOP);
        new_node->data.binop.left = left_node;
        new_node->data.binop.operator_type = POWER;
        // Recursion: Get the right side (the exponent) by calling parse_power() again - This handles right associativity of exponent operator
        new_node->data.binop.right = parse_power();
        return new_node;
    }
    // If there was no '**' symbol, just return the plain number/variable
    return left_node;
}

static ASTNode* parse_term() {
    ASTNode* node = parse_power();
    while (current_token.type == STAR || current_token.type == SLASH || current_token.type == FLOORDIV || current_token.type == MODULUS) {
        TokenType op = current_token.type;
        eat(op);
        ASTNode* new_node = create_node(AST_BINOP);
        new_node->data.binop.left = node;
        new_node->data.binop.operator_type = op;
        new_node->data.binop.right = parse_power();
        node = new_node;
    }
    return node;
}

static ASTNode* parse_expression() {
    ASTNode* node = parse_term();
    while (current_token.type == PLUS || current_token.type == MINUS) {
        TokenType op = current_token.type;
        eat(op);
        ASTNode* new_node = create_node(AST_BINOP);
        new_node->data.binop.left = node;
        new_node->data.binop.operator_type = op;
        new_node->data.binop.right = parse_term();
        node = new_node;
    }
    return node;
}

static ASTNode* parse_statement() {
    if (current_token.type == PRINT) {
        eat(PRINT);
        eat(LPAREN);
        ASTNode* node = create_node(AST_PRINT);
        node->data.print_stmt.expression = parse_expression(); 
        eat(RPAREN);
        return node;
    }
    
    // We check if it's an identifier, and then PEEK ahead to see if an '=' comes next.
    if (current_token.type == IDENTIFIER) {
        Token next = peek_next_token();
        if (next.type == ASSIGN) { 
            ASTNode* node = create_node(AST_ASSIGN);
            // Save the variable name before we eat it
            strcpy(node->data.assign.variable_name, current_token.value);
            eat(IDENTIFIER);
            eat(ASSIGN);
            // The value is whatever math comes after the '='
            node->data.assign.value = parse_expression(); 
            return node;
        }
    }
    
    // Fallback: Standalone Expression (e.g., just typing "5 + 3" or "x + 2")
    // If it's not a print or an assignment, route it straight to the math engine.
    ASTNode* node = parse_expression();
    return node;
}

void init_parser(const char* source_code) {
    init_lexer(source_code);
    advance();
}

// The main loop that builds the root of the tree
ASTNode* parse_program() {
    ASTNode* program_node = create_node(AST_PROGRAM);
    program_node->data.program.head_statement = NULL;
    // Keep track of the end of the chain to append quickly
    ASTNode* current_tail = NULL; 
    
    while (current_token.type != END_OF_FILE) {
        if (current_token.type == NEWLINE) {
            eat(NEWLINE);
            continue;
        }
         ASTNode* stmt = parse_statement();
        // If this is the very first statement, it becomes the head
        if (program_node->data.program.head_statement == NULL) {
            program_node->data.program.head_statement = stmt;
            current_tail = stmt;
        } 
        // Otherwise, add it to the end of the linked list
        else {
            current_tail->next = stmt;
            current_tail = stmt; // Move the tail tracker forward
        }
    }
    return program_node;

}
