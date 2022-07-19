#ifndef HASH_H_
#define HASH_H_

#include <inttypes.h>

// a length buffer, max length 65535
typedef struct buf_s {
    char *data;
    uint16_t len;
} buf_t;

// allocate a new buffer, copies value after the struct
buf_t *buf_new(char *v, uint16_t len);
void buf_free(buf_t **buf);

typedef struct hash_entry_s {
  char *key;
  uint64_t key_hash;
  buf_t value;
} hash_entry_t;

// create a new entry, allocating enough space for the value.
// key is a 0 terminate string
hash_entry_t *hash_entry_new_with_value(const char *key,
                                        const char *value, uint16_t value_len);
void hash_entry_free(hash_entry_t **v);

typedef struct hash_table_s {
    uint64_t total_capacity;
    uint64_t current_size;
    hash_entry_t **entries;
} hash_table_t;

hash_table_t *hash_table_new(void);
int hash_table_clear(hash_table_t *table);
void hash_table_free(hash_table_t **table);

hash_entry_t *hash_table_get(hash_table_t *table,
                       const char *key);
int hash_table_delete(hash_table_t *table, const char *key);
hash_entry_t *hash_table_set_new(hash_table_t *table, const char *key, const char *value,
                       uint16_t value_len);
int hash_table_set(hash_table_t *table, hash_entry_t *entry);


typedef enum hash_table_entry_result_e {
   ENTRY_ERROR = 0,
   ENTRY_REPLACED = 1,
   ENTRY_INSERTED = 2,
} hash_table_entry_result_t;
hash_table_entry_result_t hash_table_set_entry(hash_entry_t *entries[], uint64_t capacity, hash_entry_t *entry);

#endif // HASH_H_
