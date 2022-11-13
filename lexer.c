#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>

#include <unistd.h>

#include "lexer.h"

typedef struct
{
    uint8_t type;
    uint8_t spacer;
    uint8_t data[0];
} command_t;

typedef struct lexer_s
{
    union
    {
        uint8_t *data;
        command_t *command;
    };

    size_t capacity;
    size_t size;
    read_func_t reader;
    lexer_command *command_handler;
    lexer_command *commands;
} lexer_t;

const size_t c_lexer_buffer_size = 255;
static bool g_lexer_running = true;
static uint8_t c_lexer_spacer = '_';
static const uint32_t g_lexer_stopper = 0x0a; // big en?
static int g_lex_command_min_size = sizeof(command_t) + sizeof(g_lexer_stopper);

bool null_handler(const lexer_t *lexer)
{
    return true;
}

bool dont_check(const lexer_t *lexer)
{
    return true;
}

size_t lexer_get_data_size(const lexer_t *lexer)
{
    return lexer->size - g_lex_command_min_size;
}

const uint8_t *lexer_get_data(const lexer_t *lexer)
{
    return lexer->command->data;
}

static command_status_e _lexer_command_end(const lexer_t *lexer)
{
    /* command will not be able to terminate in the future */
    if (lexer->capacity - lexer->size < g_lex_command_min_size)
        return INVALID;

    /* command is of suitable length */
    if (lexer->size >= g_lex_command_min_size)
    {
        if (*((uint32_t *)&lexer->command->data[lexer->size - g_lex_command_min_size]) == g_lexer_stopper)
            return ASSEMBLED;
    }
    return COLLECTING;
}

static command_status_e process_command_scheme(lexer_t *lexer)
{
    command_status_e status = UNKNOWN;
    int command_type;
    bool found = false;

    if (lexer->size == 0)
    {
        status = COLLECTING;
        goto exit;
    }

    if (!lexer->command_handler)
    {
        for (command_type = 0; lexer->commands[command_type].type != 0; command_type++)
        {
            if (lexer->command->type == lexer->commands[command_type].type)
            {
                lexer->command_handler = &lexer->commands[command_type];
                break;
            }
        }

        if (!lexer->command_handler)
        {
            status = INVALID;
            goto exit;
        }
    }

    if (lexer->size < sizeof(command_t))
    {
        status = COLLECTING;
        goto exit;
    }

    if (lexer->command->spacer != c_lexer_spacer)
    {
        status = INVALID;
        goto exit;
    }

    /* have termination bytes */

    status = _lexer_command_end(lexer);
    if (status != ASSEMBLED)
    {
        goto exit;
    }

    if (lexer->command_handler->verifier && !lexer->command_handler->verifier(lexer))
    {
        status = UNVERIFIED;
        goto exit;
    }

    status = lexer->command_handler->handler(lexer) ? DONE : FAILED;
exit:
    return status;
}

void lexer_run(lexer_t *lexer)
{
    while (g_lexer_running)
    {
        int data = lexer->reader();
        command_status_e status = UNKNOWN;

        if (data == -1)
            break; // TODO: restore continue;

        if (lexer->size >= lexer->capacity)
        {
            lexer->size = 0;
            lexer->command_handler = NULL;
            // TODO: print error
        }

        lexer->size++;
        lexer->data[lexer->size - 1] = (uint8_t)data;

        status = process_command_scheme(lexer);

        switch (status)
        {
        case COLLECTING:
            continue;

        case UNKNOWN:
        case INVALID:
        case FAILED:
            printf("got bad command: 0x%02hhX (%c)\n", data, data);
            // TODO: print error
        case ASSEMBLED:
        case DONE:
            lexer->command_handler = NULL;
            lexer->size = 0;
            // TODO: print error
            break;
        }
    }
}

lexer_t *create_lexer(lexer_command *commands, read_func_t reader, size_t capacity)
{
    lexer_t *lexer = malloc(sizeof(lexer_t));
    lexer->size = 0;
    lexer->data = malloc(capacity);
    lexer->capacity = capacity;
    lexer->commands = commands;
    lexer->reader = reader;
    lexer->command_handler = NULL;

    return lexer;
}
