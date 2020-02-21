#include "const.h"
#include "sequitur.h"

// RECYCLED_LIST
SYMBOL *recycled_list;

/*
 * Symbol management.
 *
 * The functions here manage a statically allocated array of SYMBOL structures,
 * together with a stack of "recycled" symbols.
 */

/*
 * Initialization of this global variable that could not be performed in the header file.
 */
int next_nonterminal_value = FIRST_NONTERMINAL;

/**
 * Initialize the symbols module.
 * Frees all symbols, setting num_symbols to 0, and resets next_nonterminal_value
 * to FIRST_NONTERMINAL;
 */
void init_symbols(void) {
    // To be implemented.
    // frees all symbols.
    num_symbols = 0; // setting num_symbols to 0;
    next_nonterminal_value = FIRST_NONTERMINAL; // reset to FIRST_NONTERMINAL
}

/**
 * Get a new symbol.
 *
 * @param value  The value to be used for the symbol.  Whether the symbol is a terminal
 * symbol or a non-terminal symbol is determined by its value: terminal symbols have
 * "small" values (i.e. < FIRST_NONTERMINAL), and nonterminal symbols have "large" values
 * (i.e. >= FIRST_NONTERMINAL).
 * @param rule  For a terminal symbol, this parameter should be NULL.  For a nonterminal
 * symbol, this parameter can be used to specify a rule having that nonterminal at its head.
 * In that case, the reference count of the rule is increased by one and a pointer to the rule
 * is stored in the symbol.  This parameter can also be NULL for a nonterminal symbol if the
 * associated rule is not currently known and will be assigned later.
 * @return  A pointer to the new symbol, whose value and rule fields have been initialized
 * according to the parameters passed, and with other fields zeroed.  If the symbol storage
 * is exhausted and a new symbol cannot be created, then a message is printed to stderr and
 * abort() is called.
 *
 * When this function is called, if there are any recycled symbols, then one of those is removed
 * from the recycling list and used to satisfy the request.
 * Otherwise, if there currently are no recycled symbols, then a new symbol is allocated from
 * the main symbol_storage array and the num_symbols variable is incremented to record the
 * allocation.
 */

// RECYCLED_LIST

SYMBOL *new_symbol(int value, SYMBOL *rule) {

    SYMBOL *symptr = NULL;

    if ( recycled_list != NULL ) { // not empty
        symptr = recycled_list; // last symbol in recycled_list

        recycled_list = recycled_list->next; // take off last symbol on the list.

        symptr->refcnt = 0; // zeroed
        symptr->next = NULL; // zeroed
        symptr->prev = NULL; // zeroed
        symptr->nextr = NULL; // zeroed
        symptr->prevr = NULL; // zeroed

        if (value < FIRST_NONTERMINAL) {
            // rule = NULL
            symptr->rule = NULL;
            symptr->value = value;
            return symptr;
        }
        else if (value >= FIRST_NONTERMINAL) { // non-terminal

            if (rule != NULL) {
               symptr->rule = rule; // rule is non-NULL
               ref_rule(rule);
            }
            symptr->value = value;
            return symptr;
        }
    }
    else {
        // check if storage is full
        if (num_symbols >= MAX_SYMBOLS) {
            // issue a message to stderr and abort.
            fprintf(stderr, "Symbol storage is exhausted! Can not create new symbol.");
            abort();
        }
        // else
        // printf("numsymbols: %d", num_symbols);
        symptr = (symbol_storage + num_symbols); // from piazza: a[i] <--> *(a+i)
        symptr->refcnt = 0; // zeroed
        symptr->next = NULL; // zeroed
        symptr->prev = NULL; // zeroed
        symptr->nextr = NULL; // zeroed
        symptr->prevr = NULL; // zeroed
        symptr->rule = NULL;
        symptr->value = value;

        num_symbols++; // increment num_symbols global variable
    }
    return symptr;
}

/**
 * Recycle a symbol that is no longer being used.
 *
 * @param s  The symbol to be recycled.  The caller must not use this symbol any more
 * once it has been recycled.
 *
 * Symbols being recycled are added to the recycled_symbols list, where they will
 * be made available for re-allocation by a subsequent call to new_symbol.
 * The recycled_symbols list is managed as a LIFO list (i.e. a stack), using the
 * next field of the SYMBOL structure to chain together the entries.
 */
void recycle_symbol(SYMBOL *s) {
    // Add to beginning of list.
    if (recycled_list == NULL) {
        recycled_list = s;
        s->next = NULL;
    }
    else {
        // Add to list
        s->next = recycled_list->next;
        recycled_list = s;
    }

}
