/**
 * @brief vector is a dynamic, type-agnostic container module for RT-Thread that provides
 *        sequential memory access with automatic resizing capabilities. It supports various
 *        data types, dynamic memory management, and comprehensive operations including
 *        push/pop, insert/remove, sort, search, and iteration.
 *
 * @copyright Copyright (c) 2020-2025 Gary Zhang [cleancode@163.com]
 *
 * @license The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>
#include <stddef.h>
#include "rtthread.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief vector configuration for creation
 */
typedef struct {
    size_t item_size;   /* item size */
    size_t capacity;    /* capacity of the vector */
} vector_config_t;

/**
 * @brief vector handle
 */
typedef void *vector_handle_t;

/**
 * @brief vector create function
 *
 * @param config [in]: initial configuration
 * @return NULL pointer on failure or a valid vector_handle_t pointer on success
 */
vector_handle_t vector_create(const vector_config_t *config);

/**
 * @brief push data to the back
 *
 * @param handle [in]: vector handle created by vector_create
 * @param data [in]: pointer to data to be pushed
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_push_back(vector_handle_t handle, const void *data);

/**
 * @brief push multiple data to the back
 *
 * @param handle [in]: vector handle created by vector_create
 * @param block [in]: pointer to data to be pushed
 * @param num_items [in] : number of data items in block
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_push_back_block(vector_handle_t handle, const void *block, size_t num_items);

/**
 * @brief push data to the front
 *
 * @param handle [in]: vector handle created by vector_create
 * @param data [in]: pointer to data to be pushed
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_push_front(vector_handle_t handle, const void *data);

/**
 * @brief insert one item of data
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to insert
 * @param value [in]: pointer to data to insert
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_insert(vector_handle_t handle, size_t index, const void *value);

/**
 * @brief insert data block
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to insert
 * @param block [in]: pointer to data block to insert
 * @param num_items [in]: number of data items in block
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_insert_block(vector_handle_t handle, size_t index, const void *block, size_t num_items);

/**
 * @brief shrink memory size to fit current item
 *
 * @param handle [in]: vector handle created by vector_create
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_shrink(vector_handle_t handle);

/**
 * @brief get data at a specific index
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to get
 * @return void *: pointer to data retrieved
 */
void *vector_get(vector_handle_t handle, size_t index);

/**
 * @brief remove data at a specific index
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to remove
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_remove(vector_handle_t handle, size_t index);

/**
 * @brief remove data at a specific index
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to remove
 * @param length [in]: length of data block to remove
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_remove_block(vector_handle_t handle, size_t index, size_t length);

/**
 * @brief remove the last data item
 *
 * @param handle [in]: vector handle created by vector_create
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_pop_back(vector_handle_t handle);

/**
 * @brief remove the first data item
 *
 * @param handle [in]: vector handle created by vector_create
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_pop_front(vector_handle_t handle);

/**
 * @brief modify data at a specific index
 *
 * @param handle [in]: vector handle created by vector_create
 * @param index [in]: position for data to remove
 * @param data [in]: pointer to new data
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_modify(vector_handle_t handle, size_t index, void *data);

/**
 * @brief clear all items
 *
 * @param handle [in]: vector handle created by vector_create
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_clear(vector_handle_t handle);

/**
 * @brief sort items in the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @param cmp [in]: comparison function
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_sort(vector_handle_t handle, int (* cmp)(void *a, void *b));

/**
 * @brief loop over for each item in the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @param action [in]: function to perform on each item
 * @return  0 if successful, -1 otherwise
 */
int vector_for_each(vector_handle_t handle,
                    void (* action)(void *val, size_t index, size_t size, void *context),
                    void *context);

/**
 * @brief find a specified data in the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @param data [in]: data to find
 * @param compare [in]: comparation function
 * @param index [out]: item index found
 * @return 0 if success, -1 if input invalid parameter, -2 if no find.
 */
int vector_find(vector_handle_t handle,
                void *data,
                int (* compare)(const void *item, const void *data),
                uint32_t *index);

/**
 * @brief get the first item in the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @return: NULL when vector is empty, or the pointer to the firt item
 */
void *vector_front(vector_handle_t handle);

/**
 * @brief get the last item in the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @return: NULL when vector is empty, or the pointer to the las item
 */
void *vector_back(vector_handle_t handle);

/**
 * @brief get capacity
 *
 * @param handle [in]: vector handle created by vector_create
 * @return size_t: vector capacity if success, 0 if failed
 */
size_t vector_capacity(vector_handle_t handle);

/**
 * @brief get current data item count
 *
 * @param handle [in]: vector handle created by vector_create
 * @return size_t: vector size if success, 0 if failed
 */
size_t vector_size(vector_handle_t handle);

/**
 * @brief destroy the vector
 *
 * @param handle [in]: vector handle created by vector_create
 * @return 0 if success, Less than 0 indicates failed.
 */
int vector_destroy(vector_handle_t handle);


#ifdef __cplusplus
}
#endif

#endif // __VECTOR_H__
