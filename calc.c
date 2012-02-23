#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

#include "khash.h"

#define BOOL char
#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 1024

#define DONE_PARSING(p) (*p->cursor == 0)

#define IS_DIGIT(c) (c >= '0' && c <= '9')

#define IS_ALPHA(c) ((c >= 'a' && c <= 'z') || \
                     (c >= 'A' && c <= 'Z'))

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
                parser->input = str; \
                parser->cursor = str; \
                parser->nesting = 0; \
        } while(0)

KHASH_MAP_INIT_STR(str, int);
khash_t(str) *vars;

struct Parser {
        char *input;
        char *cursor;
        jmp_buf jmp_env;
        int nesting;
};

int parse_expression(struct Parser *parser);

char *chop(char *str)
{
        str[strlen(str) - 1] = 0;
        return str;
}

void print_error(struct Parser *parser, char *msg)
{
        fprintf(stderr, "%s\n", msg);
        fprintf(stderr, "\t%s\n", parser->input);

        fprintf(stderr, "\t");

        int i;
        for (i = 0; i < parser->cursor - parser->input; i++) {
                fprintf(stderr, "~");
        }
        fprintf(stderr, "^\n");
}

void set_var(char *name, int value)
{
        khiter_t k;
        int ret;

        k = kh_put(str, vars, name, &ret);
        kh_value(vars, k) = value;
}

int get_var(char *name)
{
        khiter_t k;

        k = kh_get(str, vars, name);
        if (k == kh_end(vars)) return 0;
        return kh_value(vars, k);
}

void eat_whitespace(struct Parser *parser)
{
        while (*parser->cursor == '\t' || *parser->cursor == ' ') {
                parser->cursor++;
        }
}

char *parse_symbol(struct Parser *parser)
{
        char c;
        int size = 0;
        BOOL first_char = TRUE;
        char *sym = malloc(sizeof(char) * BUFFER_SIZE);

        do {
                c = *parser->cursor;
                if (IS_ALPHA(c) || c == '_') {
                        sym[size] = c;
                        parser->cursor++;
                        size++;
                        first_char = FALSE;
                } else if (IS_DIGIT(c) && !first_char) {
                        sym[size] = c;
                        parser->cursor++;
                        size++;
                        first_char = FALSE;
                } else {
                        break;
                }
        } while (TRUE);

        if (size > 0) {
                sym[size] = 0;
                return sym;
        } else {
                if (PEEK(parser) == 0)
                        THROW_ERROR(parser, "%s", "Expected a variable, but reached the end of input");
                else
                        THROW_ERROR(parser, "Expected a variable, but got a '%c'", *parser->cursor);
        }

        return NULL;
}

int parse_number(struct Parser *parser)
{
        char c;
        int size = 0;
        char num[BUFFER_SIZE];

        do {
                c = *parser->cursor;
                if (IS_DIGIT(c)) {
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
                if (PEEK(parser) == 0)
                        THROW_ERROR(parser, "%s", "Expected a number, but reached the end of input");
                else
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
        case 0:
                THROW_ERROR(parser, "%s", "Expecting an operator but reached the end of input");
        default:
                THROW_ERROR(parser, "Expecting an operator but got '%c' instead", op);
        }
}

char parse_equal(struct Parser *parser)
{
        char c = *parser->cursor;

        if (c == '=') {
                parser->cursor++;
                return c;
        } else if (c == 0) {
                THROW_ERROR(parser, "%s", "Expecting '=' but reached the end of input");
        } else {
                THROW_ERROR(parser, "Expecting '=' but got '%c' instead", c);
        }
}

char parse_paren(struct Parser *parser)
{
        char paren = *parser->cursor;
        switch (paren) {
        case '(':
        case ')':
                parser->cursor++;
                return paren;
        case 0:
                THROW_ERROR(parser, "%s", "Expecting a paren, but reached the end of input");
        default:
                THROW_ERROR(parser, "Expecting a paren, but got '%c' instead", paren);
        }
}

int parse_variable_expr(struct Parser *parser)
{
        char *name = parse_symbol(parser);
        eat_whitespace(parser);
        int value = 0;

        char c = PEEK(parser);
        if (c == '=') {
                parse_equal(parser);
                eat_whitespace(parser);
                value = parse_expression(parser);

                set_var(name, value);
        } else {
                value = get_var(name);
        }

        //free(name);

        return value;
}

int parse_parens_expression(struct Parser *parser)
{
        char c = PEEK(parser);
        int number = 0;
        if (IS_ALPHA(c) || c == '_') {
                number = parse_variable_expr(parser);
        }
        else if (c == '(') {
                c = parse_paren(parser);
                parser->nesting++;
                eat_whitespace(parser);

                number = parse_expression(parser);

                c = parse_paren(parser);
                if (c != ')')
                        THROW_ERROR(parser, "Expecting ')', but got '%c'", c);
                parser->nesting--;
        } else {
                number = parse_number(parser);
        }

        return number;
}

int parse_uminus_expr(struct Parser *parser)
{
        if (PEEK(parser) == '-') {
                parse_operator(parser);
                eat_whitespace(parser);
                return -1 * parse_parens_expression(parser);
        } else {
                return parse_parens_expression(parser);
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

                if (op == ')' && parser->nesting)
                        return number;

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
                char op = PEEK(parser);

                if (op == ')' && parser->nesting)
                        return number;

                op = parse_operator(parser);
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

        vars = kh_init(str);

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
