#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 1024

#define DONE_PARSING(p) *p->cursor == 0

#define ERROR(parser, error_format, ...) \
        do { \
                char msg[BUFFER_SIZE]; \
                sprintf(msg, error_format, __VA_ARGS__); \
                print_error(parser, msg); \
                exit(1); \
        } while(0);



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

void eat_whitespace(struct Parser *parser)
{
        while (*parser->cursor == '\t' || *parser->cursor == ' ') {
                parser->cursor++;
        }
}

int parse_number(struct Parser *parser)
{
        char c;
        int size = 0;
        char num[BUFFER_SIZE];

        do {
                c = *parser->cursor;
                if (c >= '0' && c <= '9') {
                        num[size] = c;
                        parser->cursor++;
                        size++;
                } else {
                        break;
                }
        } while (TRUE);

        if (size > 0) {
                num[size] = 0;
                return strtol(num, NULL, 10);
        } else {
                ERROR(parser, "Expected a number, but got a '%c'", *parser->cursor);
        }

        return 0;
}

char parse_operator(struct Parser *parser)
{
        char op = *parser->cursor;

        switch (op) {
        case '+':
                parser->cursor++;
                return op;
        default:
                ERROR(parser, "I don't understand the '%c' operator", op);
        }
}

int parse_expression(struct Parser *parser)
{
        eat_whitespace(parser);

        int number = parse_number(parser);

        eat_whitespace(parser);

        if (DONE_PARSING(parser)) {
                return number;
        } else {
                char operator = parse_operator(parser);
                if (operator == '+')
                        return number + parse_expression(parser);
        }

        fprintf(stderr, "We shouldn't get here!");
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
                        result = parse_expression(&parser);
                        printf("=> %d\n", result);
                }
        }

        return 0;
}
