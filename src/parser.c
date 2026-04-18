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
        printf("Syntax Error\n");
        exit(1); 
    }
}

// Allocates memory for a new AST Node on the heap
static ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    return node;
}

// We must declare these here because they constantly call each other
static ASTNode* parse_statement();
static ASTNode* parse_comparison();
static ASTNode* parse_expression();
static ASTNode* parse_term();
static ASTNode* parse_power();
static ASTNode* parse_factor();
static ASTNode* parse_block();
static ASTNode* parse_if_statement();
static ASTNode* parse_while_statement();


static ASTNode* parse_factor() {
    Token tok = current_token;
    if (tok.type==NUMBER) {
        eat(NUMBER);
        ASTNode* node = create_node(AST_NUMBER);
        node->data.number_value = strtod(tok.value, NULL); 
        return node;
    }
    else if (tok.type == MINUS) {     // For negative numbers (Unary Minus), treat -5 as 0-5
        eat(MINUS);
        // Create a fake 0
        ASTNode* zero_node = create_node(AST_NUMBER);
        zero_node->data.number_value = 0;
        
        // Create standard subtraction: 0 - (whatever factor comes next)
        ASTNode* node = create_node(AST_BINOP);
        node->data.binop.left = zero_node;
        node->data.binop.operator_type = MINUS;
        
        node->data.binop.right = parse_factor(); 
        
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
    else if (tok.type == STRING) {
        eat(STRING);
        ASTNode* node = create_node(AST_STRING);
        strcpy(node->data.string_value, tok.value);
        return node;
    }
    else {
        printf("Syntax Error\n");
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

static ASTNode* parse_comparison() {
    ASTNode* node = parse_expression();
    if (current_token.type == LT || current_token.type == GT || current_token.type == LTE || current_token.type == GTE || current_token.type == EQEQUAL || current_token.type == NOTEQUAL) {
        TokenType op = current_token.type;
        eat(op);
        ASTNode* new_node = create_node(AST_BINOP);
        new_node->data.binop.left = node;
        new_node->data.binop.operator_type = op;
        new_node->data.binop.right = parse_expression();
        node = new_node;
    }
    return node;
}


static ASTNode* parse_block() {
    ASTNode* block_node = create_node(AST_BLOCK);
    block_node->data.block.head_statement = NULL;
    ASTNode* current_tail = NULL;

    if (current_token.type == NEWLINE) eat(NEWLINE);
    eat(INDENT);
    
    while (current_token.type != DEDENT && current_token.type != END_OF_FILE) {
        if (current_token.type == NEWLINE) {
            eat(NEWLINE);
            continue;
        }
        ASTNode* stmt = parse_statement();
        if (block_node->data.block.head_statement == NULL) {
            block_node->data.block.head_statement = stmt;
            current_tail = stmt;
        } else {
            current_tail->next = stmt;
            current_tail = stmt;
        }
    }
    
    if (current_token.type == DEDENT) {
        eat(DEDENT);
    }
    
    return block_node;
}

static ASTNode* parse_while_statement() {
    eat(WHILE);
    ASTNode* node = create_node(AST_WHILE);
    node->data.while_stmt.condition = parse_comparison();
    eat(COLON);
    node->data.while_stmt.loop_body = parse_block();
    return node;
}

static ASTNode* parse_if_statement() {
    eat(IF);
    ASTNode* node = create_node(AST_IF);
    node->data.if_stmt.condition = parse_comparison();
    node->data.if_stmt.else_body = NULL;
    
    eat(COLON);
    node->data.if_stmt.if_body = parse_block();
    
    // Treat ELIF as a nested IF statement
    if (current_token.type == ELIF) {
        current_token.type = IF;
        node->data.if_stmt.else_body = parse_if_statement();
    }
    // Standard ELSE block
    else if (current_token.type == ELSE) {
        eat(ELSE);
        eat(COLON);
        node->data.if_stmt.else_body = parse_block();
    }
    
    return node;
}

static ASTNode* parse_statement() {
     if (current_token.type == IF) {
        return parse_if_statement();
    }
    if (current_token.type == WHILE) {
        return parse_while_statement();
    }
    if (current_token.type == PRINT) {
        eat(PRINT);
        eat(LPAREN);
        ASTNode* node = create_node(AST_PRINT);
        node->data.print_stmt.expression = parse_comparison(); 
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
            node->data.assign.value = parse_comparison(); 
            return node;
        }
    }
    
    // Fallback: Standalone Expression (just typing "5 + 3" or "x + 2")
    // If it's not a print or an assignment, route it straight to the math engine.
    ASTNode* node = parse_comparison();
    return node;
}
// BODMAS Hierarchy: from lowest priority to highest: parse_comparison (<, ==) -> parse_expression (+, -) -> parse_term (*, /) -> parse_power (**) (Recursive Descent)

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

void free_ast(ASTNode* node) {
    if (node == NULL) return;

    // Recursive - Free the next statement in the linked list
    free_ast(node->next);

    // Free any children hanging off of this specific node
    switch (node->type) {
        case AST_BINOP:
            free_ast(node->data.binop.left);
            free_ast(node->data.binop.right);
            break;
        case AST_ASSIGN:
            free_ast(node->data.assign.value);
            break;
        case AST_PRINT:
            free_ast(node->data.print_stmt.expression);
            break;
        case AST_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.if_body);
            free_ast(node->data.if_stmt.else_body);
            break;
        case AST_WHILE:
            free_ast(node->data.while_stmt.condition);
            free_ast(node->data.while_stmt.loop_body);
            break;
        case AST_BLOCK:
        case AST_PROGRAM:
            free_ast(node->data.program.head_statement);
            break;
        case AST_NUMBER:
        case AST_STRING:
        case AST_IDENTIFIER: // These don't have any child pointers
            break;
    }

    // Once all the children are safely destroyed, free the node itself
    free(node);
}
