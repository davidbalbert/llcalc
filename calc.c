#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 1024

struct Parser {
        char *input;
        char *cursor;
};

char *chop(char *str)
{
        str[strlen(str) - 1] = 0;
        return str;
}


void print_error(struct Parser *parser, char *msg) {
        fprintf(stderr, "%s\n", msg);
        fprintf(stderr, "\t%s\n", parser->input);

        fprintf(stderr, "\t");

        for (int i = 0; i < parser->cursor - parser->input; i++) {
                fprintf(stderr, "~");
        }
        fprintf(stderr, "^\n");
}

int parse_number(struct Parser *parser)
{
        return 0;
}

char parse_operator(struct Parser *parser)
{
        return 0;
}

int parse_expression(struct Parser *parser)
{
        //eat_whitespace(parser);

        int left_operand = parse_number(parser);
        char operator = parse_operator(parser);
        if (operator == '+') {
                return left_operand + parse_expression(parser);
        }

        char msg[BUFFER_SIZE];
        sprintf(msg, "I don't understand the '%c' operator", operator);
        print_error(parser, msg);
        exit(1);
}

int main(int argc, const char *argv[])
{
        char input[BUFFER_SIZE];
        int result = 0;

        while(TRUE) {
                printf(">> ");
                if (fgets(input, BUFFER_SIZE, stdin)) {
                        chop(input);
                        struct Parser parser = { input, input };
                        //result = parse_expression(&parser);
                        //printf("=> %d\n", result);
                }
        }

        return 0;
}
