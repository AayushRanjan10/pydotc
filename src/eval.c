#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "eval.h"

typedef struct {
    char name[100];
    double value;
} Variable;

static Variable variables[1000];
static int num_variables = 0;

// Helper functions to set and get variables
static void set_variable(const char* name, double value)  { // if we don't use const, we might get warning on passing a string directly.
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    strcpy(variables[num_variables].name, name);
    variables[num_variables].value = value;
    num_variables++;
}

static double get_variable(const char* name) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    printf("Runtime Error\n");
    exit(1);
}

static void eval_statement(ASTNode* node);
static double eval_expression(ASTNode* node) {
    if (node->type == AST_NUMBER) {
        return node->data.number_value;
    } 
    else if (node->type == AST_IDENTIFIER) {
        return get_variable(node->data.variable_name);
    } 
    else if (node->type == AST_BINOP) {
        double left_val = eval_expression(node->data.binop.left);
        double right_val = eval_expression(node->data.binop.right);
        
        switch (node->data.binop.operator_type) {
            // Standard Math
            case PLUS:      return left_val + right_val;
            case MINUS:     return left_val - right_val;
            case STAR:      return left_val * right_val;
            case SLASH:     return left_val / right_val;
            case FLOORDIV:  return floor(left_val / right_val);
            case POWER:     return pow(left_val, right_val);
            case MODULUS:   return fmod(left_val, right_val);
            
            // Boolean Logic (Returns 1 for True, 0 for False)
            case EQEQUAL:   return left_val == right_val ? 1 : 0;
            case NOTEQUAL:  return left_val != right_val ? 1 : 0;
            case LT:        return left_val < right_val  ? 1 : 0;
            case GT:        return left_val > right_val  ? 1 : 0;
            case LTE:       return left_val <= right_val ? 1 : 0;
            case GTE:       return left_val >= right_val ? 1 : 0;
            
            default: 
                printf("Runtime Error\n"); 
                exit(1);
        }
    }
    return 0;
}

static void eval_statement(ASTNode* node) {
    if (node == NULL) return;
    if (node->type == AST_ASSIGN) {
        double val = eval_expression(node->data.assign.value);
        set_variable(node->data.assign.variable_name, val);
    } 
    else if (node->type == AST_PRINT) {
        if (node->data.print_stmt.expression->type == AST_STRING) {
            printf("%s\n", node->data.print_stmt.expression->data.string_value);
        } 
        else {
            double val = eval_expression(node->data.print_stmt.expression);
            printf("%g\n", val); // %g prints a double cleanly without trailing zeroes
        }
    } 
    else if (node->type == AST_BLOCK) {
        ASTNode* current = node->data.block.head_statement;
        while (current != NULL) {
            eval_statement(current);
            current = current->next;
        }
    } 
    else if (node->type == AST_IF) {
        if (eval_expression(node->data.if_stmt.condition) > 0) { 
            eval_statement(node->data.if_stmt.if_body);
        } 
        else if (node->data.if_stmt.else_body != NULL) {
            eval_statement(node->data.if_stmt.else_body); 
        }
    } 
    else if (node->type == AST_WHILE) {
        while (eval_expression(node->data.while_stmt.condition) > 0) {
            eval_statement(node->data.while_stmt.loop_body);
        }
    }
}

void eval_program(ASTNode* root) {
    if (root == NULL || root->type != AST_PROGRAM) return;
    
    ASTNode* current = root->data.program.head_statement;
    while (current != NULL) {
        eval_statement(current);
        current = current->next;
    }
}