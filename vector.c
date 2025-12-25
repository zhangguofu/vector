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

#include <string.h>
#include <stdlib.h>
#include "vector.h"


#define VECTOR_DEFAULT_CAPACITY (4)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct {
    size_t capacity;    /* capacity */
    size_t size;        /* actual size */
    size_t item_size;   /* element size */
    void *data;         /* memory pool for elements */
} vector_ctrl_block_t;

vector_handle_t vector_create(const vector_config_t *config)
{
    if (RT_NULL == config) {
        return (vector_handle_t)RT_NULL;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)rt_malloc(sizeof(vector_ctrl_block_t));
    if (RT_NULL == v) {
        return (vector_handle_t)RT_NULL;
    }

    if (config->capacity > 0) {
        v->capacity = config->capacity;
    } else {
        // default capacity is applied when not specified
        v->capacity = VECTOR_DEFAULT_CAPACITY;
    }

    void *buffer = (void *)rt_malloc(v->capacity * config->item_size);
    if (RT_NULL == buffer) {
        rt_free(v);
        return (vector_handle_t)RT_NULL;
    }

    v->data = buffer;
    v->item_size = config->item_size;
    v->size = 0;

    return (vector_handle_t)v;
}

int vector_push_back(vector_handle_t handle, const void *data)
{
    if (RT_NULL == handle || RT_NULL == data) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    /* double the memory size when need more space */
    if (v->size >= v->capacity) {
        size_t new_capacity = v->capacity * 2;
        void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_memcpy(new_data, v->data, v->size * v->item_size);
        rt_free(v->data);
        v->data = new_data;
        v->capacity = new_capacity;
    }

    void *dest = (uint8_t *)v->data + v->size * v->item_size;
    rt_memcpy(dest, data, v->item_size);
    v->size++;

    return 0;
}

int vector_push_front(vector_handle_t handle, const void *data)
{
    if (RT_NULL == handle || RT_NULL == data) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;

    /* double the memory size when need more space */
    if (v->size >= v->capacity) {
        size_t new_capacity = v->capacity * 2;
        void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }

        // put item at the front
        rt_memcpy(new_data, data, v->item_size);

        // copy existing data items
        void *next_pos = (uint8_t *)new_data + v->item_size;
        rt_memcpy(next_pos, v->data, v->size * v->item_size);

        // switch to new buffer
        rt_free(v->data);
        v->data = new_data;
        v->size++;
        v->capacity = new_capacity;
    } else {
        // make room for the first item
        for (size_t i = (size_t)v->size; i > 0; i--) {
            void *cur_pos = (uint8_t *)v->data + i * v->item_size;
            void *prev_pos = (uint8_t *)cur_pos - v->item_size;
            rt_memcpy(cur_pos, prev_pos, v->item_size);
        }
        // put item at the front
        rt_memcpy(v->data, data, v->item_size);
        v->size++;
    }

    return 0;
}

int vector_push_back_block(vector_handle_t handle, const void *block, size_t num_items)
{
    if ((RT_NULL == handle)
        || (RT_NULL == block)
        || (0 == num_items)) {
        return -1;
    }
    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    size_t remaining_capacity = v->capacity - v->size;
    if (num_items > remaining_capacity) {
        // double the capacity if needed
        size_t new_capacity = MAX(v->capacity * 2, (v->size + num_items) * 2);
        void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_memcpy(new_data, v->data, v->size * v->item_size);
        rt_free(v->data);
        v->data = new_data;
        v->capacity = new_capacity;
    }
    // copy the block of items to the end of the vector
    void *dest = (uint8_t *)v->data + v->size * v->item_size;
    rt_memcpy(dest, block, num_items * v->item_size);
    v->size += num_items;

    return 0;
}

int vector_insert(vector_handle_t handle, size_t index, const void *value)
{
    if (RT_NULL == handle || RT_NULL == value) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index < v->size) {
        // check available space
        if (v->size >= v->capacity) {
            // no more space to insert
            void *new_data = (void *)rt_malloc(v->size * 2 * v->item_size);
            if (RT_NULL == new_data) {
                return -3;
            }
            // copy the data items prior to index
            rt_memcpy(new_data, v->data, index * v->item_size);

            // insert value to index
            void *insert_pos = (uint8_t *)new_data + index * v->item_size;
            rt_memcpy(insert_pos, value, v->item_size);

            // copy the items behind
            void *next_pos = (uint8_t *)insert_pos + v->item_size;
            void *next_data = (uint8_t *)v->data + index * v->item_size;
            rt_memcpy(next_pos, next_data, (v->size - index) * v->item_size);

            // switch to new data
            rt_free(v->data);
            v->data = new_data;
            v->capacity = 2 * v->size;
            v->size++;
        } else {
            // make room for new item to insert
            for (size_t i = v->size; i > (size_t)index; i--) {
                void *cur_pos = (uint8_t *)v->data + i * v->item_size;
                void *prev_pos = (uint8_t *)cur_pos - v->item_size;
                rt_memcpy(cur_pos, prev_pos, v->item_size);
            }

            // insert
            void *insert_pos = (uint8_t *)v->data + index * v->item_size;
            rt_memcpy(insert_pos, value, v->item_size);
            v->size++;
        }
        return 0;
    } else {
        // index overflow
        return -2;
    }
}

int vector_insert_block(vector_handle_t handle, size_t index, const void *block, size_t num_items)
{
    if ((RT_NULL == handle)
        || (RT_NULL == block)
        || (0 == num_items)) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index < v->size) {
        // check available space
        if (v->size + num_items > v->capacity) {
            // no more space to insert
            void *new_data = (void *)rt_malloc((v->size + num_items) * 2 * v->item_size);
            if (RT_NULL == new_data) {
                return -3;
            }
            // copy the data items prior to index
            rt_memcpy(new_data, v->data, index * v->item_size);

            // insert block to index
            void *insert_pos = (uint8_t *)new_data + index * v->item_size;
            rt_memcpy(insert_pos, block, num_items * v->item_size);

            // copy the items behind
            void *next_pos = (uint8_t *)insert_pos + num_items * v->item_size;
            void *next_data = (uint8_t *)v->data + index * v->item_size;
            rt_memcpy(next_pos, next_data, (v->size - index) * v->item_size);

            // switch to new data
            rt_free(v->data);
            v->data = new_data;
            v->capacity = 2 * (v->size + num_items);
            v->size += num_items;
        } else {
            // make room for new block to insert
            for (size_t i = v->size - 1; i >= (size_t)index; i--) {
                void *cur_pos = (uint8_t *)v->data + i * v->item_size;
                void *new_pos = (uint8_t *)cur_pos + num_items * v->item_size;
                rt_memcpy(new_pos, cur_pos, v->item_size);
            }
            // insert the block of data
            void *insert_pos = (uint8_t *)v->data + index * v->item_size;
            rt_memcpy(insert_pos, block, num_items * v->item_size);
            v->size += num_items;
        }
        return 0;
    } else {
        // index overflow
        return -2;
    }
}

void *vector_get(vector_handle_t handle, size_t index)
{
    if (RT_NULL == handle) {
        return RT_NULL;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index < v->size) {
        return (uint8_t *)v->data + index * v->item_size;
    } else {
        return RT_NULL;
    }
}

int vector_remove(vector_handle_t handle, size_t index)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index >= v->size) {
        return -2;
    }

    // compute pointer to item to be removed
    void *item = (uint8_t *)v->data + index * v->item_size;

    // compute pointer to next item after the one to be removed
    void *next_item = (uint8_t *)v->data + (index + 1) * v->item_size;

    // compute number of bytes to shift
    size_t num_bytes = (v->size - (index + 1)) * v->item_size;

    // shift items after the removed item down by one position
    if (num_bytes > 0) {
        rt_memmove(item, next_item, num_bytes);
    }

    // decrement vector size
    v->size--;

    // if vector is less than half full, reduce its capacity
    if ((v->capacity > VECTOR_DEFAULT_CAPACITY) && (v->capacity > 2 * v->size)) {
        size_t new_capacity = MAX(v->capacity / 2, VECTOR_DEFAULT_CAPACITY);
        void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_memcpy(new_data, v->data, v->size * v->item_size);
        rt_free(v->data);
        v->data = new_data;
        v->capacity = new_capacity;
    }

    return 0;
}

int vector_remove_block(vector_handle_t handle, size_t index, size_t length)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index + length > v->size) {
        return -3;
    }

    // compute pointer to item to be removed
    void *item = (uint8_t *)v->data + index * v->item_size;

    // compute pointer to next item after the last one to be removed
    void *next_item = (uint8_t *)v->data + (index + length) * v->item_size;

    // compute number of bytes to shift
    size_t num_bytes = (v->size - (index + length)) * v->item_size;

    // shift items after the removed item down by one position
    if (num_bytes > 0) {
        rt_memmove(item, next_item, num_bytes);
    }

    // decrement vector size
    v->size -= length;

    // if vector is less than half full, reduce its capacity
    if ((v->capacity > VECTOR_DEFAULT_CAPACITY) && (v->capacity > 2 * v->size)) {
        size_t new_capacity = MAX(v->capacity / 2, VECTOR_DEFAULT_CAPACITY);
        void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_memcpy(new_data, v->data, v->size * v->item_size);
        rt_free(v->data);
        v->data = new_data;
        v->capacity = new_capacity;
    }

    return 0;
}

int vector_pop_back(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->size > 0) {
        v->size--;
        if ((v->capacity > VECTOR_DEFAULT_CAPACITY) && (v->capacity > 2 * v->size)) {
            size_t new_capacity = MAX(v->capacity / 2, VECTOR_DEFAULT_CAPACITY);
            void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
            if (RT_NULL == new_data) {
                return -2;
            }
            rt_memcpy(new_data, v->data, v->size * v->item_size);
            rt_free(v->data);
            v->data = new_data;
            v->capacity = new_capacity;
        }
    }

    return 0;
}

int vector_pop_front(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->size > 0) {
        // move one step backwards
        for (size_t i = 1; i < v->size; i++) {
            void *cur_pos = (uint8_t *)v->data + i * v->item_size;
            void *prev_pos = (uint8_t *)cur_pos - v->item_size;
            rt_memcpy(prev_pos, cur_pos, v->item_size);
        }

        // update
        v->size--;
        if ((v->capacity > VECTOR_DEFAULT_CAPACITY) && (v->capacity > 2 * v->size)) {
            size_t new_capacity = MAX(v->capacity / 2, VECTOR_DEFAULT_CAPACITY);
            void *new_data = (void *)rt_malloc(new_capacity * v->item_size);
            if (RT_NULL == new_data) {
                return -2;
            }
            rt_memcpy(new_data, v->data, v->size * v->item_size);
            rt_free(v->data);
            v->data = new_data;
            v->capacity = new_capacity;
        }
    }

    return 0;
}

int vector_modify(vector_handle_t handle, size_t index, void *data)
{
    if (RT_NULL == handle) {
        return -1;
    }

    if (RT_NULL == data) {
        return -2;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (index < v->size) {
        rt_memcpy((uint8_t *)v->data + index * v->item_size, data, v->item_size);
    } else {
        return -3;
    }

    return 0;
}

int vector_clear(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->capacity > VECTOR_DEFAULT_CAPACITY) {
        void *new_data = (void *)rt_malloc(VECTOR_DEFAULT_CAPACITY * v->item_size);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_free(v->data);
        v->data = new_data;
        v->capacity = VECTOR_DEFAULT_CAPACITY;
    }

    v->size = 0;
    return 0;
}

static int vector_merge_sort(void *data_list, size_t n, size_t size, int (* cmp)(void *a, void *b))
{
    // check if any of the inputs are invalid
    if (data_list == NULL || n <= 0 || size <= 0 || cmp == NULL) {
        return -1;
    }

    // cast the data list to a char pointer to allow easy memory arithmetic
    char *arr = (char *)data_list;
    char *mem_buf = (char *)rt_malloc(n * size);
    if (mem_buf == NULL) {
        return -2;
    }

    // Perform merge sort using a divide-and-conquer approach
    for (size_t step = 1; step < n; step <<= 1) {
        for (size_t i = 0; i < n; i += 2 * step) {
            size_t left = i;
            size_t mid = i + step;
            size_t right = i + 2 * step;

            if (mid > n) {
                mid = n;
            }

            if (right > n) {
                right = n;
            }

            size_t i1 = left;
            size_t i2 = mid;
            size_t j = 0;

            // merge the two sub-lists by comparing elements using the comparison function
            while (i1 < mid && i2 < right) {
                if (cmp(arr + i1 * size, arr + i2 * size) <= 0) {
                    rt_memcpy(mem_buf + j * size, arr + i1 * size, size);
                    i1++;
                } else {
                    rt_memcpy(mem_buf + j * size, arr + i2 * size, size);
                    i2++;
                }
                j++;
            }

            // copy any remaining elements from the left sub-list
            while (i1 < mid) {
                rt_memcpy(mem_buf + j * size, arr + i1 * size, size);
                i1++;
                j++;
            }

            // copy any remaining elements from the right sub-list
            while (i2 < right) {
                rt_memcpy(mem_buf + j * size, arr + i2 * size, size);
                i2++;
                j++;
            }

            // copy the sorted sub-list from the temporary buffer to the original list
            rt_memcpy(arr + left * size, mem_buf, (right - left) * size);
        }
    }

    rt_free(mem_buf);
    return 0;
}

int vector_sort(vector_handle_t handle, int (* cmp)(void *a, void *b))
{
    if (RT_NULL == handle || RT_NULL == cmp) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    return vector_merge_sort(v->data, v->size, v->item_size, cmp);
}

int vector_for_each(vector_handle_t handle,
                    void (* action)(void *val, size_t index, size_t size, void *context),
                    void *context)
{
    if (RT_NULL == handle || RT_NULL == action) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->size > 0) {
        /* go over each element */
        for (size_t i = 0; i < v->size; i++) {
            void *val = (uint8_t *)v->data + i * v->item_size;
            action(val, i, v->size, context);
        }
    } else {
        /* no element */
        action(RT_NULL, 0, 0, context);
    }

    return 0;
}

int vector_shrink(vector_handle_t handle)
{
    if (handle == RT_NULL) {
        return -1;
    }
    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if ((v->capacity > VECTOR_DEFAULT_CAPACITY) && (v->capacity > v->size)) {
        size_t reduced_capacity = MAX(v->size, VECTOR_DEFAULT_CAPACITY);
        void *new_data = rt_malloc(v->item_size * reduced_capacity);
        if (RT_NULL == new_data) {
            return -2;
        }
        rt_memcpy(new_data, v->data, v->size * v->item_size);
        rt_free(v->data);
        v->data = new_data;
        v->capacity = reduced_capacity;
    }

    return 0;
}

int vector_find(vector_handle_t handle,
                void *data,
                int (* compare)(const void *item, const void *data),
                uint32_t *index)
{
    if ((RT_NULL == handle)
        || (RT_NULL == data)
        || (RT_NULL == compare)
        || (RT_NULL == index)) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    // traverse the data array and compare each element with the input data
    for (uint32_t i = 0; i < v->size; i++) {
        void *item = (void *)((uint8_t *)v->data + i * v->item_size);
        if (0 == compare(item, data)) {
            *index = i;
            return 0;
        }
    }

    // not found
    return -2;
}

void *vector_front(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return RT_NULL;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->size > 0) {
        return v->data;
    } else {
        return RT_NULL;
    }
}

void *vector_back(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return RT_NULL;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    if (v->size > 0) {
        return (void *)((uint8_t *)v->data + (v->size - 1) * v->item_size);
    } else {
        return RT_NULL;
    }
}

size_t vector_capacity(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return 0;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    return v->capacity;
}

size_t vector_size(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return 0;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    return v->size;
}

int vector_destroy(vector_handle_t handle)
{
    if (RT_NULL == handle) {
        return -1;
    }

    vector_ctrl_block_t *v = (vector_ctrl_block_t *)handle;
    v->size = 0;
    v->capacity = 0;
    rt_free(v->data);
    v->data = RT_NULL;
    rt_free(v);

    return 0;
}
