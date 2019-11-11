#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

struct list_t;

typedef size_t index_t;
#define INDEX_T "%lu"
typedef float listelem_t;
#define LISTELEM_T "%f"
typedef char state_t;
#define STATE_T "%d"

/**
 * Init list_t
 * @param self List
 * @param size Size of list
 * @return Error code
 */
int ListInit(list_t &self, index_t size);

/**
 * Finalize list_t
 * @param self List
 * @return Error code
 */
int ListFinalize(list_t &self);

/**
 * Check list_t for errors
 * @param self List
 * @return State of list
 */
state_t ListOk(const list_t &self);

/**
 * Add item to list
 * @param self List
 * @param absIndex Absolute index of previous element
 * @param data Data to add
 * @return Error code
 */
int ListAddAbs(list_t &self, index_t absIndex, listelem_t data);

/**
 * Remove item from list
 * @param self List
 * @param absIndex Absolute index of item to delete
 * @return Error code
 */
int ListRemoveAbs(list_t &self, index_t absIndex);

/**
 * Add item to list
 * @param self List
 * @param index Index of previous element
 * @param data Data to add
 * @return Error code
 */
int ListAdd(list_t &self, index_t index, listelem_t data);

/**
 * Remove item from list
 * @param self List
 * @param index Index of item to delete
 * @return Error code
 */
int ListRemove(list_t &self, index_t index);

/**
 * Sort list
 * @param self List
 * @return Error code
 */
int ListSort(list_t &self);


/**
 * Realloc new space for list if it's full
 * @param self List
 * @return Error code
 */
int ListExtend_(list_t &self);

/**
 * Init free items in list
 * @param self List
 * @param absStart Absolute index of first free item
 * @param absEnd Absolute index of last free item
 * @return Error code
 */
int ListInitFree_(list_t &self, index_t absStart, index_t absEnd);

/**
 * Calc new size of list
 * @param size Current size of list
 * @return New size
 */
index_t ListNextSize_(index_t size);

/**
 * Check element if it's free
 * @param self List
 * @param absIndex Absolute index of item to check
 * @return 1 if free, 0 if not free
 */
int ListIsFree_(const list_t &self, index_t absIndex);

/**
 * Find absolute index of item
 * @param self List
 * @param index Index of item
 * @return Absolute index of item
 */
index_t ListGetAbsIndex_(const list_t &self, index_t index);