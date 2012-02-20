#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

#define BOOL char
#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 1024

#define DONE_PARSING(p) (*p->cursor == 0)

#define PEEK(p) (*p->cursor)

#define THROW_ERROR(parser, error_format, ...) \
        do { \
                char msg[BUFFER_SIZE]; \
                sprintf(msg, error_format, __VA_ARGS__); \
                print_error(parser, msg); \
                longjmp(parser->jmp_env, 1); \
        } while(0)

#define TRY_ERROR(parser) if (!setjmp(parser->jmp_env))

#define INIT_PARSER(parser, str) \
        do { \
                parser->str = input; \
                parser->cursor = input; \
        } while(0)

struct Parser {
        char *input;
        char *cursor;
        jmp_buf jmp_env;
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
                THROW_ERROR(parser, "Expected a number, but got a '%c'", *parser->cursor);
        }

        return 0;
}

char parse_operator(struct Parser *parser)
{
        char op = *parser->cursor;

        switch (op) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
                parser->cursor++;
                return op;
        default:
                THROW_ERROR(parser, "Expecting an operator but got '%c' instead", op);
        }
}

int parse_uminus_expr(struct Parser *parser)
{
        if (PEEK(parser) == '-') {
                parse_operator(parser);
                eat_whitespace(parser);
                return -1 * parse_number(parser);
        } else {
                return parse_number(parser);
        }
}

int parse_pow_expr(struct Parser *parser)
{
        int number = parse_uminus_expr(parser);

        eat_whitespace(parser);

        if (DONE_PARSING(parser))
                return number;

        if (PEEK(parser) != '^')
                return number;

        // we already know the next operator is a '^'
        // so just eat it up
        parse_operator(parser);
        eat_whitespace(parser);

        return pow(number, parse_pow_expr(parser));
}

int parse_times_expr(struct Parser *parser)
{
        int number = parse_pow_expr(parser);

        eat_whitespace(parser);

        while(!DONE_PARSING(parser)) {
                char op = PEEK(parser);
                if (op == '+' || op == '-')
                        return number;
                op = parse_operator(parser);
                eat_whitespace(parser);
                if (op == '*')
                        number *= parse_pow_expr(parser);
                else if (op == '/')
                        number /= parse_pow_expr(parser);
                eat_whitespace(parser);
        }

        return number;
}

int parse_expression(struct Parser *parser)
{
        eat_whitespace(parser);

        int number = parse_times_expr(parser);

        eat_whitespace(parser);

        while(!DONE_PARSING(parser)) {
                char op = parse_operator(parser);
                eat_whitespace(parser);
                if (op == '+')
                        number += parse_times_expr(parser);
                else if (op == '-')
                        number -= parse_times_expr(parser);
                eat_whitespace(parser);
        }
        return number;
}


int main(int argc, const char *argv[])
{
        char input[BUFFER_SIZE];
        int result = 0;

        while(TRUE) {
                printf(">> ");
                if (fgets(input, BUFFER_SIZE, stdin)) {
                        chop(input);
                        if (strlen(input) == 0) {
                                continue;
                        }

                        struct Parser *parser = malloc(sizeof(struct Parser));
                        INIT_PARSER(parser, input);

                        TRY_ERROR(parser) {
                                result = parse_expression(parser);
                                printf("=> %d\n", result);
                        }

                        free(parser);
                }
        }

        return 0;
}
