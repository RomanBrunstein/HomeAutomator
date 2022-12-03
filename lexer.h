#ifndef __LEXER_H__
#define __LEXER_H__
#include <stdint.h>
#include <stdbool.h>

typedef struct lexer_s lexer_t;

typedef enum
{
    UNKNOWN = 0,
    INVALID,
    COLLECTING,
    ASSEMBLED,
    UNVERIFIED,
    DONE,
    FAILED,
} command_status_e;

typedef int (*read_func_t)();
typedef bool (*syntax_checker)(const lexer_t *lexer);
typedef bool (*command_handler)(const lexer_t *lexer);

typedef struct
{
    uint8_t type;
    syntax_checker verifier;
    command_handler handler;
} lexer_command;

/* getters */
size_t lexer_get_data_size(const lexer_t *lexer);
const uint8_t *lexer_get_data(const lexer_t *lexer);

/* stub handlers */
bool null_handler(const lexer_t *lexer);
bool dont_check(const lexer_t *lexer);

lexer_t *create_lexer(lexer_command *commands, read_func_t reader, size_t capacity);
void lexer_run(lexer_t *lexer);

#endif /* __LEXER_H__ */