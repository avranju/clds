// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "c_logging/logger.h"

#include "c_pal/threadapi.h"
#include "c_pal/timer.h"
#include "c_pal/gballoc_hl.h"
#include "c_pal/gballoc_hl_redirect.h"

#include "clds/clds_singly_linked_list.h"
#include "clds_singly_linked_list_perf.h"

#define THREAD_COUNT 10
#define INSERT_COUNT 1000

typedef struct TEST_ITEM_TAG
{
    char key[20];
} TEST_ITEM;

DECLARE_SINGLY_LINKED_LIST_NODE_TYPE(TEST_ITEM);

typedef struct THREAD_DATA_TAG
{
    CLDS_SINGLY_LINKED_LIST_HANDLE singly_linked_list;
    CLDS_SINGLY_LINKED_LIST_ITEM* items[INSERT_COUNT];
    double runtime;
    CLDS_HAZARD_POINTERS_THREAD_HANDLE clds_hazard_pointers_thread;
} THREAD_DATA;

static int insert_thread(void* arg)
{
    size_t i;
    THREAD_DATA* thread_data = arg;
    int result;

    double start_time = timer_global_get_elapsed_ms();
    for (i = 0; i < INSERT_COUNT; i++)
    {
        if (clds_singly_linked_list_insert(thread_data->singly_linked_list, thread_data->clds_hazard_pointers_thread, thread_data->items[i]) != 0)
        {
            LogError("Error inserting");
            break;
        }
    }

    if (i < INSERT_COUNT)
    {
        LogError("Error in test");
        result = MU_FAILURE;
    }
    else
    {
        thread_data->runtime = timer_global_get_elapsed_ms() - start_time;
        result = 0;
    }

    return result;
}

static int delete_thread(void* arg)
{
    size_t i;
    THREAD_DATA* thread_data = arg;
    int result;

    double start_time = timer_global_get_elapsed_ms();
    for (i = 0; i < INSERT_COUNT; i++)
    {
        if (clds_singly_linked_list_delete(thread_data->singly_linked_list, thread_data->clds_hazard_pointers_thread, thread_data->items[i]) != CLDS_SINGLY_LINKED_LIST_DELETE_OK)
        {
            LogError("Error deleting");
            break;
        }
    }

    if (i < INSERT_COUNT)
    {
        LogError("Error in test");
        result = MU_FAILURE;
    }
    else
    {
        thread_data->runtime = timer_global_get_elapsed_ms() - start_time;
        result = 0;
    }

    return result;
}

int clds_singly_linked_list_perf_main(void)
{
    CLDS_HAZARD_POINTERS_HANDLE clds_hazard_pointers;
    CLDS_SINGLY_LINKED_LIST_HANDLE singly_linked_list;
    THREAD_HANDLE threads[THREAD_COUNT];
    THREAD_DATA* thread_data;
    size_t i;
    size_t j;

    clds_hazard_pointers = clds_hazard_pointers_create();
    if (clds_hazard_pointers == NULL)
    {
        LogError("Error creating hazard pointers");
    }
    else
    {
        singly_linked_list = clds_singly_linked_list_create(clds_hazard_pointers);
        if (singly_linked_list == NULL)
        {
            LogError("Error creating singly linked list");
        }
        else
        {
            thread_data = malloc_2(THREAD_COUNT, sizeof(THREAD_DATA));
            if (thread_data == NULL)
            {
                LogError("Error allocating thread data array");
            }
            else
            {
                for (i = 0; i < THREAD_COUNT; i++)
                {
                    thread_data[i].singly_linked_list = singly_linked_list;
                    thread_data[i].clds_hazard_pointers_thread = clds_hazard_pointers_register_thread(clds_hazard_pointers);
                    if (thread_data[i].clds_hazard_pointers_thread == NULL)
                    {
                        LogError("Error registering thread with harzard pointers");
                        break;
                    }
                    else
                    {
                        for (j = 0; j < INSERT_COUNT; j++)
                        {
                            thread_data[i].items[j] = CLDS_SINGLY_LINKED_LIST_NODE_CREATE(TEST_ITEM, NULL, NULL);
                            if (thread_data[i].items[j] == NULL)
                            {
                                LogError("Error allocating test item");
                                break;
                            }
                            else
                            {
                                TEST_ITEM* test_item = CLDS_SINGLY_LINKED_LIST_GET_VALUE(TEST_ITEM, thread_data[i].items[j]);
                                (void)sprintf(test_item->key, "%zu_%zu", i, j);
                            }
                        }

                        if (j < INSERT_COUNT)
                        {
                            size_t k;

                            for (k = 0; k < j; k++)
                            {
                                CLDS_SINGLY_LINKED_LIST_NODE_RELEASE(TEST_ITEM, thread_data[i].items[k]);
                            }
                            break;
                        }
                    }
                }

                if (i < THREAD_COUNT)
                {
                    LogError("Error creating test thread data");
                }
                else
                {
                    // insert test
                    LogInfo("Start insert test");

                    for (i = 0; i < THREAD_COUNT; i++)
                    {
                        if (ThreadAPI_Create(&threads[i], insert_thread, &thread_data[i]) != THREADAPI_OK)
                        {
                            LogError("Error spawning test thread");
                            break;
                        }
                    }

                    if (i < THREAD_COUNT)
                    {
                        for (j = 0; j < i; j++)
                        {
                            int dont_care;
                            (void)ThreadAPI_Join(threads[j], &dont_care);
                        }
                    }
                    else
                    {
                        bool is_error = false;
                        double runtime = 0.0;

                        for (i = 0; i < THREAD_COUNT; i++)
                        {
                            int thread_result;
                            (void)ThreadAPI_Join(threads[i], &thread_result);
                            if (thread_result != 0)
                            {
                                is_error = true;
                            }
                            else
                            {
                                runtime += thread_data[i].runtime;
                            }
                        }

                        if (!is_error)
                        {
                            LogInfo("Insert test done in %.02f ms, %.02f inserts/s/thread, %.02f inserts/s on all threads",
                                runtime,
                                ((double)THREAD_COUNT * (double)INSERT_COUNT) / (double)runtime * 1000.0,
                                ((double)THREAD_COUNT * (double)INSERT_COUNT) / ((double)runtime / THREAD_COUNT) * 1000.0);

                            // delete test

                            for (i = 0; i < THREAD_COUNT; i++)
                            {
                                if (ThreadAPI_Create(&threads[i], delete_thread, &thread_data[i]) != THREADAPI_OK)
                                {
                                    LogError("Error spawning test thread");
                                    break;
                                }
                            }

                            if (i < THREAD_COUNT)
                            {
                                for (j = 0; j < i; j++)
                                {
                                    int dont_care;
                                    (void)ThreadAPI_Join(threads[j], &dont_care);
                                }
                            }
                            else
                            {
                                is_error = false;
                                runtime = 0;

                                for (i = 0; i < THREAD_COUNT; i++)
                                {
                                    int thread_result;
                                    (void)ThreadAPI_Join(threads[i], &thread_result);
                                    if (thread_result != 0)
                                    {
                                        is_error = true;
                                    }
                                    else
                                    {
                                        runtime += thread_data[i].runtime;
                                    }
                                }

                                if (!is_error)
                                {
                                    LogInfo("Delete test done in %.02f ms, %.02f deletes/s/thread, %.02f deletes/s on all threads",
                                        runtime,
                                        ((double)THREAD_COUNT * (double)INSERT_COUNT) / (double)runtime * 1000.0,
                                        ((double)THREAD_COUNT * (double)INSERT_COUNT) / ((double)runtime / THREAD_COUNT) * 1000.0);
                                }
                            }
                        }
                    }

                    for (i = 0; i < THREAD_COUNT; i++)
                    {
                        clds_hazard_pointers_unregister_thread(thread_data[i].clds_hazard_pointers_thread);
                    }

                    free(thread_data);
                }
            }

            clds_singly_linked_list_destroy(singly_linked_list);
        }

        clds_hazard_pointers_destroy(clds_hazard_pointers);
    }

    return 0;
}
