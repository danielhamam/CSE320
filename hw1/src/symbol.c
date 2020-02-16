#include "const.h"
#include "sequitur.h"

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

// RECYCLED_LIST
SYMBOL *recycled_list;


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
SYMBOL *new_symbol(int value, SYMBOL *rule) {
    // To be implemented.
    // SYMBOL newsymbol = *(symbol_storage + num_symbols);
    // SYMBOL *ptr = &newsymbol;
    // return ptr;
    // first, check if recycled symbols is empty. DONT USE PREV for recycled_symbols
    // CHECK IF TWO MEMBERS IN THE STACK:
    if ( recycled_list != NULL && (*recycled_list).next != NULL) {
        // its not empty (LIFO REMEMBER);
        SYMBOL *temp = recycled_list; // made pointer to preserve recycled_list
        while( (*temp).next != NULL) {
            temp = temp->next;
        }
        // temp refers to last node in recycled_list (LIFO)
        SYMBOL *allocated_symbol = temp; // used to satisfy the request. removed from recycling list
        temp--; // go to previous node
        temp->next = NULL; // deleted last node

        (*allocated_symbol).refcnt = 0; // zeroed
        (*allocated_symbol).next = 0; // zeroed
        (*allocated_symbol).prev = 0; // zeroed
        (*allocated_symbol).nextr = 0; // zeroed
        (*allocated_symbol).prevr = 0; // zeroed

        if (value < FIRST_NONTERMINAL) {
            // rule = NULL
            (*allocated_symbol).rule = NULL;
            (*allocated_symbol).value = value;
            return allocated_symbol;
        }
        else if (value >= FIRST_NONTERMINAL) {
            // it is nonterminal
            // rule is non-NULL
            if (rule != NULL) {
                (*allocated_symbol).rule = rule;
            }
            (*allocated_symbol).value = value;
            return allocated_symbol;
        }
    }
    else if (recycled_list != NULL) {
        // ONLY ONE MEMBER IN STACK. REMOVE AND ITS EMPTY.
        SYMBOL *allocated_symbol = recycled_list;
        recycled_list = NULL; // now is empty (removed LIFO)

        (*allocated_symbol).refcnt = 0; // zeroed
        (*allocated_symbol).next = 0; // zeroed
        (*allocated_symbol).prev = 0; // zeroed
        (*allocated_symbol).nextr = 0; // zeroed
        (*allocated_symbol).prevr = 0; // zeroed

        if (value < FIRST_NONTERMINAL) {
            // rule = NULL
            (*allocated_symbol).rule = NULL;
            (*allocated_symbol).value = value;
            return allocated_symbol;
        }
        else if (value >= FIRST_NONTERMINAL) {
            // it is nonterminal
            // rule is non-NULL
            if (rule != NULL) {
                (*allocated_symbol).rule = NULL;
            }
            (*allocated_symbol).value = value;
            return allocated_symbol;
        }
    }
    else {
    // recycled_list == NULL
        // check if storage is full
        if (num_symbols >= MAX_SYMBOLS) {
            // issue a message to stderr and abort.
            fprintf(stderr, "Symbol storage is exhausted! Can not create new symbol.");
            abort();
        }
    // else
    // printf("numsymbols: %d", num_symbols);
    SYMBOL newsymbol = *(symbol_storage + num_symbols); // from piazza: a[i] <--> *(a+i)
    newsymbol.refcnt = 0; // zeroed
    newsymbol.next = 0; // zeroed
    newsymbol.prev = 0; // zeroed
    newsymbol.nextr = 0; // zeroed
    newsymbol.prevr = 0; // zeroed
    newsymbol.rule = NULL;
    newsymbol.value = value;
    SYMBOL *symptr = &newsymbol;
    *(symbol_storage + num_symbols) = newsymbol; // move this symbol into the array.
    num_symbols++; // increment num_symbols global variable.
    // printf("THIS VALUE: %d", symptr->value);
    return symptr;
}
    return NULL;
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
    (*s).next = (recycled_list); // now it is the head of recycled_list
    recycled_list = s; // switch to now this being head.

    // // Num_symbols lower.
    // num_symbols--;
}
