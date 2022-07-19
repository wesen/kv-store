#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include <assert.h>

#include "./fnv.h"
#include "./hash.h"

// allocate a new buffer, copies value after the struct
buf_t *buf_new(char *v, uint16_t len) {
    buf_t *res = malloc(sizeof(buf_t) + len);
    if (res != NULL) {
        res->data = (char *)res + sizeof(buf_t);
        res->len = len;
        memcpy(res->data, v, res->len);
    }
    return res;
}

void buf_free(buf_t **buf) {
    if (buf != NULL) {
        if (*buf != NULL) {
          free(*buf);
        }
        *buf = NULL;
    }
}

// create a new entry, allocating enough space for the value.
// key is a 0 terminate string
hash_entry_t *hash_entry_new_with_value(const char *key,
                                        const char *value, uint16_t value_len) {
    const uint16_t key_len = strlen(key) + 1;
    hash_entry_t *res = malloc(sizeof(hash_entry_t) + key_len + value_len);
    if (res != NULL) {
        res->key = (char *)res + sizeof(hash_entry_t);
        strncpy(res->key, key, key_len);
        res->key_hash = fnv_hash(key);
        res->value.len = value_len;
        res->value.data = (char *)res + sizeof(hash_entry_t) + key_len;
        memcpy(res->value.data, value, res->value.len);
    }
    return res;
}

void hash_entry_free(hash_entry_t **v) {
    if (v != NULL) {
        if (*v != NULL) {
            free(*v);
        }
        *v = NULL;
    }
}

hash_table_t *hash_table_new(void) {
    const uint64_t INITIAL_CAPACITY = 1024;
    hash_entry_t **entries = malloc(INITIAL_CAPACITY * sizeof(hash_entry_t*));
    if (entries == NULL) {
        return NULL;
    }
    hash_table_t *ret = malloc(sizeof(hash_table_t));
    if (ret != NULL) {
        ret->total_capacity = INITIAL_CAPACITY;
        ret->current_size = 0;
        ret->entries = entries;
    }
    return ret;
}

void hash_table_free(hash_table_t **table) {
    if (table != NULL) {
        if (*table != NULL) {
            hash_entry_t **entries = (*table)->entries;
            for (uint64_t i = 0; i < (*table)->total_capacity; i++) {
                if (entries[i] != NULL) {
                    hash_entry_free(&entries[i]);
                }
            }
            free(*table);
        }
        *table = NULL;
    }
}

hash_entry_t *hash_table_get(hash_table_t *table, const char *key) {
  if (table == NULL || key == NULL) {
    return NULL;
  }
  const uint64_t h = fnv_hash(key);

  for (uint64_t i = 0; i < table->total_capacity; i++) {
      const uint64_t idx = (i + h) % table->total_capacity;
      // found at least one empty entry, so that means the key is just not here
      if (table->entries[i] == NULL) {
          return NULL;
      }
      if (table->entries[i]->key_hash == h) {
          return table->entries[i];
      }
  }

  return NULL;
}

hash_entry_t *hash_table_set_new(hash_table_t *table,
                             const char *key,
                             const char *value, uint16_t len) {
    hash_entry_t *entry = hash_entry_new_with_value(key, value, len);
    if (hash_table_set(table, entry)) {
        return entry;
    } else {
        hash_entry_free(&entry);
        return NULL;
    }
}

int hash_table_set(hash_table_t *table, hash_entry_t *entry) {
    if (table == NULL || entry == NULL) {
        printf("NULL\n");
        return 0;
    }

    if (table->current_size > (table->total_capacity / 2)) {
        printf("resize\n");
        // resize
        const uint64_t new_capacity = table->total_capacity * 2;
        hash_entry_t **new_entries = malloc(new_capacity * sizeof(hash_entry_t*));
        if (new_entries == NULL) {
        printf("no new entries\n");
            return 0;
        }
        for (uint64_t i = 0; i < table->total_capacity; i++) {
            if (table->entries[i] != NULL) {
                hash_table_entry_result_t ret = hash_table_set_entry(new_entries, new_capacity, table->entries[i]);
                if (ret != ENTRY_INSERTED) {
                    assert("Could not insert entry into bigger array");
                    return 0;
                }
            }
        }
        free(table->entries);
        table->entries = new_entries;
        table->total_capacity = new_capacity;
        printf("resized\n");
    }

    hash_table_entry_result_t ret = hash_table_set_entry(table->entries, table->total_capacity, entry);
    printf("ret: %d, entries: %p, capacity: %d, size: %d\n", ret, table->entries, table->total_capacity, table->current_size);
    switch (ret) {
        case ENTRY_INSERTED:
            table->current_size++;
            return 1;
        case ENTRY_REPLACED:
            return 1;
        case ENTRY_ERROR:
        default:
            return 0;
    }

    return 0;
}

hash_table_entry_result_t hash_table_set_entry(
    hash_entry_t *entries[], uint64_t total_capacity,
    hash_entry_t *entry) {
    for (uint64_t i = 0; i < total_capacity; i++) {
        const uint64_t idx = (i + entry->key_hash) % total_capacity;
        // found at least one empty entry, so that means the key is just not
        // here
        if (entries[i] == NULL) {
            entries[i] = entry;
            return ENTRY_INSERTED;
        }
        if (entries[i]->key_hash == entry->key_hash) {
            // key already exists!
            printf("%s=%s already exists!\n", entries[i]->key, entry->key);
            hash_entry_free(&entries[i]);
            entries[i] = entry;
            return ENTRY_REPLACED;
        }
    }

    return ENTRY_ERROR;
}
