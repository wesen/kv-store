#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "hash.h"

#define INPUTSIZE 1024
#define INPUTSIZE 1024

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros

typedef enum commands_e {
  COMMAND_SET = 's',
  COMMAND_GET = 'g',
  COMMAND_DELETE = 'd',
  COMMAND_CLEAR = 'c'
} commands_t;

// reads a list of `key\nvalue\n` entries
int main(int argc, char *argv[])
{
    char line[INPUTSIZE + 1] = {0};
	char key[INPUTSIZE + 1] = {0};
	char value[INPUTSIZE + 1] = {0};
    char command[2] = {0};

    hash_table_t *table = hash_table_new();

    int ret = 0;
    do {
        char *s = fgets(line, INPUTSIZE, stdin);
        if (s == NULL || s[0] == '\0') {
            break;
        }

        if (line[0] == COMMAND_SET) {
            int ret = sscanf(line, "%1s %" STR(INPUTSIZE) "[^\n ] %" STR(INPUTSIZE) "[^\n ]", command, key, value);
            if (ret == 3) {
                hash_entry_t *entry = hash_table_set_new(table, key, value, strlen(value));
                if (entry == NULL) {
                    printf("Could not add %s=%s\n", key, value);
                } else {
                    printf("Added %s=%s\n", key, value);
                }
            }
        } else if (line[0] == COMMAND_GET) {
            int ret = sscanf(line, "%1s %" STR(INPUTSIZE) "[^\n ]", command, key);
            if (ret == 2) {
                hash_entry_t *entry = hash_table_get(table, key);
                if (entry == NULL) {
                    printf("Could not find entry %s\n", key);
                } else {
                    printf("%s=%s\n", key, entry->value.data);
                }
            }
        } else if (line[0] == COMMAND_CLEAR) {
            printf("clear command not implemented\n");
        } else if (line[0] == COMMAND_DELETE) {
            printf("delete command not implemented\n");
        } else {
            printf("unknown command %c\n", line[0]);
        }
    } while (!feof(stdin));
    printf("ret: %d\n", ret);

    hash_table_free(&table);

    return 0;
}
