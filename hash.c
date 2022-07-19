#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <assert.h>

#undef DEBUG_LEVEL
#define DEBUG_LEVEL 0

#include "./helpers.h"
#include "./fnv.h"
#include "./hash.h"

// resize at 2/3
#define RESIZE_CAPACITY_THRESHOLD(cap) ((cap) * 2 / 3)

static const uint64_t INITIAL_CAPACITY = 1024;

// allocate a new buffer, copies value after the struct
buf_t *buf_new(char *v, uint16_t len) {
    buf_t *res = malloc(sizeof(buf_t) + len);
    if (res != NULL) {
        res->data = (char *) res + sizeof(buf_t);
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
        res->key = (char *) res + sizeof(hash_entry_t);
        strncpy(res->key, key, key_len);
        res->key_hash = fnv_hash(key);
        res->deleted = 0;
        res->value.len = value_len;
        res->value.data = (char *) res + sizeof(hash_entry_t) + key_len;
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

int hash_table_init(hash_table_t *table) {
    if (table != NULL) {
        hash_entry_t **entries = malloc(INITIAL_CAPACITY * sizeof(hash_entry_t *));
        if (entries == NULL) {
            return 0;
        }
        memset(entries, 0, sizeof(hash_entry_t *) * INITIAL_CAPACITY);
        table->total_capacity = INITIAL_CAPACITY;
        table->current_size = 0;
        table->entries = entries;

        return 1;
    }

    return 0;
}

hash_table_t *hash_table_new(void) {
    hash_table_t *ret = malloc(sizeof(hash_table_t));
    if (ret != NULL) {
        if (!hash_table_init(ret)) {
            free(ret);
            return NULL;
        }
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

int hash_table_clear(hash_table_t *table) {
    if (table != NULL) {
        if (table->entries != NULL) {
            for (uint64_t i = 0; i < table->total_capacity; i++) {
                if (table->entries[i] != NULL) {
                    hash_entry_free(&table->entries[i]);
                    table->current_size--;
                    assert(table->current_size >= 0);
                }
            }
            assert(table->current_size == 0);

            free(table->entries);
            table->entries = NULL;
            table->total_capacity = 0;
        }
        return hash_table_init(table);
    }
    return 0;
}

hash_entry_t *hash_table_get(hash_table_t *table, const char *key) {
    if (table == NULL || key == NULL) {
        return NULL;
    }
    const uint64_t h = fnv_hash(key);
    DEBUG_PRINT("get key %llx, total_capacity %lld\n", h, table->total_capacity);

    for (uint64_t i = 0; i < table->total_capacity; i++) {
        const uint64_t idx = (i + h) % table->total_capacity;
        DEBUG_PRINT("test at idx %lld %p\n", idx, table->entries[idx]);
        // found at least one empty entry, so that means the key is just not here
        if (table->entries[idx] == NULL) {
            return NULL;
        }
        uint64_t entry_hash = table->entries[idx]->key_hash;
        DEBUG_PRINT("hash at idx %lld: %llx, looking for %llx\n", idx, entry_hash, h);
        if (entry_hash == h) {
            if (table->entries[idx]->deleted == 0) {
                return table->entries[idx];
            } else {
                return NULL;
            }
        }
    }

    return NULL;
}

// 0: B=b DELETED
// 1: A=a // not C but not empty
// 2: C=c // C -> return
// 3: 0

// add A:a -> hash(A) = 1
// add B:b -> hash(B) = 0
// add C:c -> hash(C) = 0
// get C = c
// delete B
// get C -> FAIL

int hash_table_delete(hash_table_t *table, const char *key) {
    if (table == NULL || key == NULL) {
        return 0;
    }
    const uint64_t h = fnv_hash(key);

    for (uint64_t i = 0; i < table->total_capacity; i++) {
        const uint64_t idx = (i + h) % table->total_capacity;
        // found at least one empty entry, so that means the key is just not here
        if (table->entries[idx] == 0) {
            return 0;
        }
        if (table->entries[idx]->key_hash == h && table->entries[idx]->deleted == 0) {
            table->entries[idx]->deleted = 1;
            table->current_size--;
            assert(table->current_size >= 0);
            return 1;
        }
    }

    return 0;
}

hash_entry_t *hash_table_set_new(hash_table_t *table,
                                 const char *key,
                                 const char *value, uint16_t len) {
    hash_entry_t *entry = hash_entry_new_with_value(key, value, len);
    if (entry == NULL) {
        return NULL;
    }
    switch (hash_table_set(table, entry)) {
        case ENTRY_NOP:
        case ENTRY_ERROR:
            hash_entry_free(&entry);
            return NULL;
        case ENTRY_REPLACED:
        case ENTRY_INSERTED:
            return entry;
    }

    assert(0); // unreachable
}

hash_table_entry_result_t hash_table_set(hash_table_t *table, hash_entry_t *entry) {
    if (table == NULL || entry == NULL) {
        return ENTRY_NOP;
    }

    if (table->current_size > (RESIZE_CAPACITY_THRESHOLD(table->total_capacity))) {
        const uint64_t new_capacity = table->total_capacity * 2;
        DEBUG_PRINT("resize from %lld to %lld\n", table->current_size, new_capacity);

        hash_entry_t **new_entries = malloc(new_capacity * sizeof(hash_entry_t *));
        if (new_entries == NULL) {
            return ENTRY_ERROR;
        }
        memset(new_entries, 0, sizeof(hash_entry_t *) * new_capacity);
        for (uint64_t i = 0; i < table->total_capacity; i++) {
            if (table->entries[i] != NULL) {
                if (table->entries[i]->deleted) {
                    hash_entry_free(&table->entries[i]);
                } else {
                    hash_table_entry_result_t ret = hash_table_set_entry(new_entries, new_capacity, table->entries[i]);
                    if (ret != ENTRY_INSERTED) {
                        assert("Could not insert entry into bigger array");
                        return ENTRY_ERROR;
                    }
                }
            }
        }
        free(table->entries);
        table->entries = new_entries;
        table->total_capacity = new_capacity;
    }

    hash_table_entry_result_t ret =
            hash_table_set_entry(
                    table->entries,
                    table->total_capacity,
                    entry);
    if (ret == ENTRY_INSERTED) {
        table->current_size++;
    }
    DEBUG_PRINT("ret: %d, entries: %p, capacity: %lld, size: %lld\n",
                ret, table->entries, table->total_capacity, table->current_size);
    return ret;
}

hash_table_entry_result_t hash_table_set_entry(
        hash_entry_t *entries[], uint64_t total_capacity,
        hash_entry_t *new_entry) {
    for (uint64_t i = 0; i < total_capacity; i++) {
        const uint64_t idx = (i + new_entry->key_hash) % total_capacity;
        // found at least one empty new_entry, so that means the key is just not
        // here
        hash_entry_t *current_entry = entries[idx];
        if (current_entry == NULL) {
            DEBUG_PRINT("insert %llx into idx %lld\n", new_entry->key_hash, idx);
            entries[idx] = new_entry;
            return ENTRY_INSERTED;
        }
        if (current_entry->key_hash == new_entry->key_hash) {
            // key already exists!
            DEBUG_PRINT("replace %llx into idx %lld\n", new_entry->key_hash, idx);
            const int was_deleted = current_entry->deleted;
            hash_entry_free(&entries[idx]);
            entries[idx] = new_entry;
            return was_deleted ? ENTRY_INSERTED : ENTRY_REPLACED;
        }
    }

    return ENTRY_ERROR;
}
