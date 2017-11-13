/* Name: Anshuman Biswal */
#include <stdlib.h>
#include <string.h>
#include "vector.h"
static int auto_capacity(vector *v, size_t *new_capacity);
/**
 * @brief Creates a new vector object
 * @param[in] item_size The size of each element in bytes
 * @param[in] capacity Default number of items allocated to the vector
 * @return A pointer to the new vector object, or NULL on failure
 * @remarks If capacity is 0, an empty vector will be created
 */
 vector *vector_create(size_t item_size, size_t capacity)
{
    vector *v = malloc(sizeof *v);
    
    if (v != NULL) {
        v->base = NULL;
        v->size = 0;
        v->capacity = capacity;
        v->item_size = item_size;
        
        if (capacity > 0) {
            /* Allocate the default capacity */
            v->base = malloc(capacity * item_size);
            
            if (v->base == NULL) {
                /* Clean up rather than leaving a zombie */
                free(v);
                v = NULL;
            }
        }
    }
    
    return v;
}
/**
 * @brief Creates an exact copy of vector object
 * @param[in] v The vector being copied
 * @return A pointer to the new vector object, or NULL on failure
 */
 vector *vector_clone(vector *v)
{
    vector *p = vector_create(v->item_size, v->capacity);
    
    if (p != NULL) {
        /* Copy the parts that vector_create() didn't */
        memcpy(p->base, v->base, v->size * v->item_size);
        p->size = v->size;
    }
    
    return p;
}
/**
 * @brief Clears a vector object created by vector_new() to an empty state
 * @param[in] v A pointer to the vector being destroyed
 */
void vector_clear(vector *v)
{
    v->size = 0;
    v->capacity = 0;
    free(v->base);
}
/**
 * @brief Resizes a vector object to the specified capacity
 * @param[in] v A pointer to the vector being resized
 * @param[in] capacity The new capacity
 * @return True/non-zero if the resize succeeded, false/zero otherwise
 * @remarks 1) The new capacity may be larger or smaller
 *          2) No-op if the capacity is unchanged, with a success return
 *          3) Resizing may invalidate pointers into the vector 
 */
int vector_resize(vector *v, size_t capacity)
{
    if (capacity == 0)
        return 0;
        
    if (capacity != v->capacity) {
        void *temp;
            
        v->capacity = capacity;
        
        /*
            If the vector is empty, realloc() depends on v->base 
            being initialized to NULL
        */
        temp = realloc(v->base, v->capacity * v->item_size);
        
        if (temp == NULL)
            return 0;
            
        v->base = temp;
    }
    
    return 1;
}
/**
 * @brief Returns the size of the specified vector
 * @param[in] v A pointer to the vector
 * @return The number of items inserted into the vector
 */
 size_t vector_size(vector *v)
{
    return v->size;
}
/**
 * @brief Returns the item stored at the specified index of a vector
 * @param[in] v A pointer to the vector
 * @param[in] index The index of the requested item
 * @return A generic pointer to the item
 */
 void *vector_at(vector *v, size_t index)
{
    return &((char*)v->base)[index * v->item_size];
}
/**
 * @brief Inserts a single item in a vector at the specified index
 * @param[in] v A pointer to the vector being inserted into
 * @param[in] item A pointer to the item being appended
 * @param[in] index The index where the item will be inserted
 * @return True/non-zero if the insert succeeded, false/zero otherwise
 * @remarks 1) The vector may be resized to make room
 *          2) The item pointed to is copied by value into the vector
 *          3) The size of the vector will increase by 1 on success
 *          4) The capacity of the vector may increase by more than 1
 *          5) All items from index to v->size may be shifted to make room
 */
 int vector_insert(vector *v, void *item, size_t index)
{
    void *src, *dst;
    
    if (index > v->size)
        return 0;
    
    if (v->size == v->capacity) {
        /* Resize to the next auto-growth amount */
        size_t new_capacity;
        
        if (!auto_capacity(v, &new_capacity) || 
            !vector_resize(v, new_capacity))
        {
            return 0;
        }
            
        v->capacity = new_capacity;
    }
    
    src = vector_at(v, index + 1);
    dst = vector_at(v, index);
    
    /* Make room for a new item */
    memmove(src, dst, (v->size - index) * v->item_size);
    
    /* Insert the new item */
    memcpy(dst, item, v->item_size);
    ++v->size;
    
    return 1;
}
/**
 * @brief Removes the specified item within the a vector
 * @param[in] v A pointer to the vector
 * @param[in] index The index of the item
 * @return True/non-zero if the value was found and removed, false/zero otherwise
 * @remarks All items following the found value may be shifted to fill in the hole
 */
int vector_remove(vector *v, size_t index)
{
    if (index >= v->size)
        return 0;
    else if (index == v->size - 1) {
        /* Special case: no copy when removing the last item */
        --v->size;
    }
    else {
        void *dst = vector_at(v, index);
        void *src = vector_at(v, index + 1);
        
        /* Fill in the vacated slot */
        memmove(dst, src, (v->size - index - 1) * v->item_size);
        --v->size;
    }
    
    return 1;
}
/**
 * @brief Removes the specified item within the a vector
 * @param[in] v A pointer to the vector
 * @param[in] pred A predicate for determining the first matching item
 * @return True/non-zero if the value was found and removed, false/zero otherwise
 * @remarks All items following the found value may be shifted to fill in the hole
 */
int vector_remove_if(vector *v, int (*pred)(void *item))
{
    size_t index = vector_find_if(v, pred);
    
    if (index != -1)
        return vector_remove(v, index);
        
    return 0;
}
/**
 * @brief Searches for the specified value within the a vector
 * @param[in] v A pointer to the vector
 * @param[in] item A pointer to the value being searched for
 * @return The index of the found value, or (size_t)-1 if not found
 * @remarks A byte-by-byte comparison is used to test equality
 */
size_t vector_find(vector *v, void *value)
{
    size_t i;
    
    for (i = 0; i < v->size; i++) {
        if (memcmp(vector_at(v, i), value, v->item_size) == 0)
            return i;
    }
    
    return -1;
}
/**
 * @brief Searches for the specified value within the a vector
 * @param[in] v A pointer to the vector
 * @param[in] pred A predicate for determining the first matching item
 * @return The index of the found value, or (size_t)-1 if not found
 */
size_t vector_find_if(vector *v, int (*pred)(void *item))
{
    size_t i;
    
    for (i = 0; i < v->size; i++) {
        if (pred(vector_at(v, i)))
            return i;
    }
    
    return -1;
}
/**
 * @brief Calculates the auto-growth of a vector
 * @param[in] v A pointer to the vector
 * @param[out] new_capacity The calculated new capacity
 * @return True/non-zero if the vector can grow, false/zero otherwise
 */
static int auto_capacity(vector *v, size_t *new_capacity)
{
    if (v->capacity == -1)
        return 0;
    else if (v->capacity == 0) {
        /* Default to something reasonable for an empty vector */
        *new_capacity = 4;
    }
    else {
        /* Try to grow by 50% of the current capacity */
        *new_capacity = v->capacity * 1.5;
        
        if (*new_capacity < v->capacity) {
            /* Max out the capacity if 50% growth overflows (wraps) */
            *new_capacity = -1;
        }
    }
    
    return 1;
}