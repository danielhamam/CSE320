#include "const.h"
#include "sequitur.h"

/*
 * Digram hash table.
 *
 * Maps pairs of symbol values to first symbol of digram.
 * Uses open addressing with linear probing.
 * See, e.g. https://en.wikipedia.org/wiki/Open_addressing
 */

/**
 * Clear the digram hash table.
 */
void init_digram_hash(void) {
    // To be implemented.
    int dig_index = 0;
    debug("init_digram_hash");
    while (dig_index != MAX_DIGRAMS) {
        *(digram_table + dig_index) = NULL;
        dig_index++;
    }
}

/**
 * Look up a digram in the hash table.
 *
 * @param v1  The symbol value of the first symbol of the digram.
 * @param v2  The symbol value of the second symbol of the digram.
 * @return  A pointer to a matching digram (i.e. one having the same two
 * symbol values) in the hash table, if there is one, otherwise NULL.
 */
SYMBOL *digram_get(int v1, int v2) {
    debug("DIGRAM_GET()");
    // To be implemented.
    int original = DIGRAM_HASH(v1, v2);
    // Check original
    SYMBOL *orig_digram = *(digram_table + original);
    if (orig_digram == NULL) {
        return NULL;
    }
    else if (orig_digram != NULL && orig_digram != TOMBSTONE) {
        if (orig_digram->value == v1 && orig_digram->next->value == v2)
            return orig_digram;
    }
    debug("INSIDE DIGRAM_GET");
    int index = original + 1;
    while (index != original) {
        // debug("index is %d", index);
        SYMBOL *found_digram = *(digram_table + index);
        if (found_digram == NULL) {
            return NULL;
        }
        else if (index == MAX_DIGRAMS) {
            index = 0;
        }
        else if (found_digram == TOMBSTONE) {
            index++;
        }
        else if (found_digram->value == v1 && found_digram->next->value == v2) {
            debug("DIGRAM_GET --> FOUND DIAGRAM & RETURNED");
            return found_digram;
        }
        index++;
    }
    debug("DIGRAM_GET --> NOT FOUND");
    return NULL; // did not find
}

/**
 * Delete a specified digram from the hash table.
 *
 * @param digram  The digram to be deleted.
 * @return 0 if the digram was found and deleted, -1 if the digram did
 * not exist in the table.
 *
 * Note that deletion in an open-addressed hash table requires that a
 * special "tombstone" value be left as a replacement for the value being
 * deleted.  Tombstones are treated as vacant for the purposes of insertion,
 * but as filled for the purpose of lookups.
 *
 * Note also that this function will only delete the specific digram that is
 * passed as the argument, not some other matching digram that happens
 * to be in the table.  The reason for this is that if we were to delete
 * some other digram, then somebody would have to be responsible for
 * recycling the symbols contained in it, and we do not have the information
 * at this point that would allow us to be able to decide whether it makes
 * sense to do it here.
 */
int digram_delete(SYMBOL *digram) {

    debug("DIGRAM_DELETE");

    // To be implemented.
    int original = DIGRAM_HASH(digram->value, digram->next->value);

    SYMBOL *orig_digram = *(digram_table + original);
    if (orig_digram != NULL) {
        debug("DIGRAM_DELETE BLOCK 0");
        if (orig_digram == digram && orig_digram->next == digram->next) {
            *(digram_table + original) = TOMBSTONE;
            return 0;
        }
    }

    int index = original + 1;

    while (index != original) {
        SYMBOL *found_digram = *(digram_table + index);
        if (found_digram == NULL) {
            debug("DIGRAM_DELETE BLOCK 1");
            return -1;
        }
        else if (found_digram == TOMBSTONE) {
            index++;
        }
        else if (found_digram == digram && found_digram->next == digram->next) {
            // we now found it, so delete:
            *(digram_table + index) = TOMBSTONE;
            return 0;
        }
        else if (index == MAX_DIGRAMS) {
            index = 0;
        }
        index++;
    }
    return -1; // digram did not exist in the table.
}

/**
 * Attempt to insert a digram into the hash table.
 *
 * @param digram  The digram to be inserted.
 * @return  0 in case the digram did not previously exist in the table and
 * insertion was successful, 1 if a matching digram already existed in the
 * table and no change was made, and -1 in case of an error, such as the hash
 * table being full or the given digram not being well-formed.
 */
int digram_put(SYMBOL *digram) {

    // First, check if it exists:
    debug("DIGRAM->value: %d", digram->value);
    debug("DIGRAM->next->value: %d", digram->next->value);

    SYMBOL *ptr1 = digram_get(digram->value, digram->next->value); // Giving Error
    if (ptr1 != NULL) {
        debug("DIGRAM ALREADY FOUND");\
        return 1; // already found.
    }

    int original = DIGRAM_HASH(digram->value, digram->next->value);
    int index = original + 1;
    SYMBOL *orig_digram = *(digram_table + original);
    if (orig_digram == NULL || orig_digram == TOMBSTONE) {
        debug("DIGRAM PUT AT INDEX: %d", original);
        *(digram_table + original) = digram;
        return 0;
    }

    // Second, we can insert into table (because not found previously);
    while (index != original) {
        SYMBOL *found_digram = *(digram_table + index);
        if (found_digram == NULL || found_digram == TOMBSTONE) {
            *(digram_table + index) = digram;
            debug("DIGRAM PUT AT INDEX: %d", index);
            return 0; // did not previoiusly exist in the table and insertion was successful.
        }
        if (index == MAX_DIGRAMS) {
            index = 0;
        }
        index++;
    }
    debug("DIGRAM NOT PLACED");
    return -1; // digram did not exist in the table.
}
