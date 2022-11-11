
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

char *MAC1 = "1ad2a3ad1adffb";

char data[] = "l_\x01\x02\x0a\0\0\0" // l command
              "p_\x0a\0\0\0"         // p command
              "d_\x0a\0\0\0";        // unknown command
int read_wrapper()
{
    static int i = 0;
    return i < sizeof(data) - 1 ? (int)data[i++] : -1;
}

bool wifi_send(void *channel, void *source, size_t size)
{
    printf("wifi sent %hX\n", (int)(*(uint16_t *)source));
    return true;
}

typedef struct
{
    uint8_t a : 1, b : 1, c : 1, d : 1, e : 1, f : 1, g : 1, h : 1, i : 1, j : 1, k : 1;
} lamps_t;

static bool l_handler(const lexer_t *lexer)
{
    size_t size = lexer_get_data_size(lexer);
    const uint8_t *data = lexer_get_data(lexer);

    return wifi_send(MAC1, (void *)data, size);
}

static bool l_verifiy(const lexer_t *lexer)
{
    size_t size = lexer_get_data_size(lexer);

    if (size != 2)
    {
        // error
        return false;
    }
    return true;
}

static bool p_handler(const lexer_t *lexer)
{
    printf("hi I'm p command\n");
    return true;
}

lexer_command commands[] = {{.type = (uint8_t)'l', l_verifiy, l_handler},
                            {.type = (uint8_t)'p', dont_check, p_handler},
                            NULL};
int main()
{
    lexer_t *lexer = create_lexer(commands, read_wrapper, 255);
    lexer_run(lexer);
    return 0;
}