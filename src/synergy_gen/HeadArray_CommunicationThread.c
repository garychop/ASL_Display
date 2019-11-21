/* generated thread source file - do not edit */
#include "HeadArray_CommunicationThread.h"

TX_THREAD HeadArray_CommunicationThread;
void HeadArray_CommunicationThread_create(void);
static void HeadArray_CommunicationThread_func(ULONG thread_input);
static uint8_t HeadArray_CommunicationThread_stack[2048] BSP_PLACE_IN_SECTION_V2(".stack.HeadArray_CommunicationThread") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
TX_QUEUE q_HeadArrayCommunicationQueue;
static uint8_t queue_memory_q_HeadArrayCommunicationQueue[1600];
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void HeadArray_CommunicationThread_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_q_HeadArrayCommunicationQueue;
    err_q_HeadArrayCommunicationQueue = tx_queue_create (&q_HeadArrayCommunicationQueue, (CHAR *) "New Queue", 16,
                                                         &queue_memory_q_HeadArrayCommunicationQueue,
                                                         sizeof(queue_memory_q_HeadArrayCommunicationQueue));
    if (TX_SUCCESS != err_q_HeadArrayCommunicationQueue)
    {
        tx_startup_err_callback (&q_HeadArrayCommunicationQueue, 0);
    }

    UINT err;
    err = tx_thread_create (&HeadArray_CommunicationThread, (CHAR *) "HeadArray_CommunicationThread",
                            HeadArray_CommunicationThread_func, (ULONG) NULL, &HeadArray_CommunicationThread_stack,
                            2048, 1, 1, 1, TX_AUTO_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&HeadArray_CommunicationThread, 0);
    }
}

static void HeadArray_CommunicationThread_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    HeadArray_CommunicationThread_entry ();
}
