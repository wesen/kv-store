#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#include "hash.h"

#define INPUTSIZE 1024

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros

#define TEST_STRING_COUNT  1000000
#define MAX_KEY_LENGTH  512
#define MAX_VALUE_LENGTH  512

// TODO:
//
// x implement delete
// x implement clear
// - implement random tester
// - implement snapshot loading/saving
//
// - implement on disk log
// - implement tombstone marking
// - implement on disk compaction
// - implement checksum / crash recovery
//
// - implement zeromq client/server
//
// fast rand from https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
static unsigned long x=123456789, y=362436069, z=521288629;

// fast rand
unsigned long xorshf96(void) {          //period 2^96-1
unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}

typedef enum commands_e {
  COMMAND_SET = 's',
  COMMAND_GET = 'g',
  COMMAND_DELETE = 'd',
  COMMAND_CLEAR = 'c'
} commands_t;

typedef enum random_action_e {
  ACTION_SET = 0,
  ACTION_GET ,
  ACTION_CLEAR ,
  ACTION_DELETE ,
  ACTION_ERROR
} random_action_t;

typedef struct random_action_distribution_entry_s {
    random_action_t action;
    uint16_t weight;
} random_action_distribution_entry_t;

typedef struct random_action_distribution_s {
    uint16_t total_weight;
    random_action_distribution_entry_t *entries;
} random_action_distribution_t;

void random_action_distribution_init(random_action_distribution_t *distribution) {
    distribution->total_weight = 0;
    for (int i = 0; distribution->entries[i].weight > 0; i++) {
        distribution->total_weight += distribution->entries[i].weight;
    }
}

random_action_t random_action_distribution_get_random_action(random_action_distribution_t *distribution) {
    int n  = xorshf96() % distribution->total_weight + 1;
    int acc = 0;
    for (int i = 0; distribution->entries[i].weight > 0 && acc <= distribution->total_weight; i++) {
        acc += distribution->entries[i].weight;
        if (acc > n) {
            return distribution->entries[i].action;
        }
    }

    return ACTION_ERROR;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

static void hexdump(char *data, unsigned long len) {
  unsigned long i;
  for (i = 0; i < len; i += 16) {
    unsigned long j;
    fprintf(stderr, "%06lx    ", i);
    for (j = 0; j < min(len - i, 16); j++)
      fprintf(stderr, "%02x ", data[i + j]);
    fprintf(stderr, "\n");
  }
}


int main(int argc, char *argv[])
{
    printf("foobar\n");
    char line[INPUTSIZE + 1] = {0};
	char key[INPUTSIZE + 1] = {0};
	char value[INPUTSIZE + 1] = {0};
    char command[2] = {0};

    hash_table_t *table = hash_table_new();

    if (argc > 1) {
        if (strcmp(argv[1], "random") == 0) {
            // pin down seed
            srand(0);

            random_action_distribution_entry_t entries[] = {
                {
                    .action = ACTION_SET,
                    .weight = 50,
                },
                {.action = ACTION_GET, .weight = 50},
                {.action = ACTION_DELETE, .weight = 10},
                {.action = ACTION_CLEAR, .weight = 1},
                {.action = ACTION_ERROR, .weight = 0}

            };

            random_action_distribution_t distribution =
                {
                .total_weight =  0,
                .entries =  entries
            };
            random_action_distribution_init(&distribution);

            // to fuzz, we would have to read this from stdin
            char *keys = malloc(sizeof(char[TEST_STRING_COUNT][MAX_KEY_LENGTH]));
            assert(keys != NULL);

            int *value_lengths = malloc(sizeof(int[TEST_STRING_COUNT]));
            assert(value_lengths != NULL);
            memset(value_lengths, 0, sizeof(int[TEST_STRING_COUNT]));

            int *key_set = malloc(sizeof(int[TEST_STRING_COUNT]));
            assert(key_set!= NULL);
            memset(key_set, 0, sizeof(int[TEST_STRING_COUNT]));

            char *values = malloc(sizeof(char[TEST_STRING_COUNT][MAX_VALUE_LENGTH]));
            assert(values != NULL);

            for (int i = 1; i < TEST_STRING_COUNT; i++) {
                // leave enough room to copy i
                int klen = xorshf96() % (MAX_KEY_LENGTH - 1 - sizeof(i)) + sizeof(i) + 1;
                int vlen = xorshf96() % (MAX_VALUE_LENGTH - 1);
                value_lengths[i] = vlen;
                /* printf("%d: klen: %d\n", i, klen); */
                assert(klen >= sizeof(i) + 1);

                for (int c = 0; c < klen; c++) {
                    keys[i * MAX_KEY_LENGTH + c] = xorshf96() % 254 + 1;
                }
                // to ensure there are no key collisions in our random keys,
                // we copy i at the beginning of keys, avoiding an initial 0
                memcpy(keys + 1, &i, sizeof(i));
                keys[i * MAX_KEY_LENGTH + klen] = '\0';
                /* hexdump(&keys[i * MAX_KEY_LENGTH], klen); */
                assert(strlen(&keys[i * MAX_KEY_LENGTH]) > 0);

                for (int c = 0; i < vlen; i++) {
                    values[i * MAX_VALUE_LENGTH + c] = xorshf96() % 255;
                }
            }

            printf("initialized\n");

            for (int i = 0; i < 1000; i++) {
                int idx = xorshf96() % TEST_STRING_COUNT;
                switch (random_action_distribution_get_random_action(&distribution)) {
                    case ACTION_SET:
                    {
                        printf("%d set %d keylen %ld\n", i, idx, strlen(&keys[idx * MAX_KEY_LENGTH]));
                        hash_entry_t *entry =
                            hash_table_set_new(table,
                                               &keys[idx * MAX_KEY_LENGTH],
                                               &values[idx * MAX_VALUE_LENGTH], value_lengths[idx]);
                        assert(entry != NULL);
                        key_set[idx] = 1;
                        hexdump(entry->key, strlen(entry->key));
                        hash_entry_t *e2 = hash_table_get(table, &keys[idx * MAX_KEY_LENGTH]);
                        printf("entry: %p keylen %ld, hash, %llx, e2 %p\n", entry, strlen(entry->key), entry->key_hash, e2);
                        assert(entry == e2);
                    }
                    break;

                    case ACTION_GET:
                    {
                        printf("get %d idx\n", idx);

                    }
                    break;

                    case ACTION_DELETE:
                    {
                        printf("delete %d idx\n", idx);
                    }
                    break;

                    case ACTION_CLEAR:
                    {
                        printf("clear\n");
                    }
                    break;

                    default:
                        assert(0 && "Could not get random action");
                        break;
                }
            }

            free(values);
            free(keys);
        } else {
            printf("Unknown mode: %s\n", argv[1]);
        }
    } else {
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
                        printf("Added %s=%s, current_size: %lld\n", key, value, table->current_size);
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
                int ret = hash_table_clear(table);
                printf("table cleared: %d\n", ret);
            } else if (line[0] == COMMAND_DELETE) {
                int ret = sscanf(line, "%1s %" STR(INPUTSIZE) "[^\n ]", command, key);
                if (ret == 2) {
                    int ret = hash_table_delete(table, key);
                    printf("deleted %s, current_size %lld, ret: %d\n", key, table->current_size, ret);
                }

            } else {
                printf("unknown command %c\n", line[0]);
            }
        } while (!feof(stdin));
        printf("ret: %d\n", ret);
    }


    hash_table_free(&table);

    return 0;
}