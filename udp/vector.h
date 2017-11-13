/* Name: Anshuman Biswal */
#ifndef AB_VECTOR_H
#define AB_VECTOR_H
#include <stddef.h>
/**
 * @brief Returns the value of an item at the specified index
 * @remarks This is a convenience wrapper around vector_at() for the average use case
 */
#define VECTOR_AT(T,v,i) *((T*)vector_at((v), (i)))
/**
 * @brief Inserts a value of type T into a vector at the specified index
 * @remarks This is a convenience wrapper to support rvalues.
 *          Note that VECTOR_INSERT() cannot be used as an expression
 */
#define VECTOR_INSERT(T, v, x, i) do {        \
    T __ab_var26121981 = x;                 \
    vector_insert(v, &__ab_var26121981, i); \
} while (0)
/**
 * @brief A structure representing the vector object
 */
typedef struct vector {
    void   *base;      /**< Raw memory for items */
    size_t  size;      /**< The number of inserted items */
    size_t  capacity;  /**< The number of potential items before a resize */
    size_t  item_size; /**< The number of bytes occupied by an item */
} vector;
extern vector *vector_create(size_t item_size, size_t capacity);
extern vector *vector_clone(vector *v);
extern void    vector_clear(vector *v);
extern int     vector_resize(vector *v, size_t capacity);
extern size_t  vector_size(vector *v);
extern void   *vector_at(vector *v, size_t index);
extern int     vector_insert(vector *v, void *item, size_t index);
extern int     vector_remove(vector *v, size_t index);
extern int     vector_remove_if(vector *v, int (*pred)(void *item));
extern size_t  vector_find(vector *v, void *value);
extern size_t  vector_find_if(vector *v, int (*pred)(void *item));
#endif