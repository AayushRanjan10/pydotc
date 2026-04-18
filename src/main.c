#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "eval.h"

int main(int argc, char** argv) {
    // Ensure the user provided a filename
    if (argc < 2) {
        printf("Usage: ./pydotc <filename.py>\n");
        return 1;
    }

    // Open the file
    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    // Create a massive string (capable of holding 64 KB of text)
    static char source_code[65536]; 
    
    // Read the file into our string array using fread and null-terminate it
    size_t bytes_read = fread(source_code, 1, sizeof(source_code) - 1, file);
    source_code[bytes_read] = '\0'; 
    
    fclose(file);

    // Run the interpreter
    init_parser(source_code);
    ASTNode* root = parse_program();

    eval_program(root);

    free_ast(root);

    return 0;
}
