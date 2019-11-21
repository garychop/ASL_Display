/* generated thread header file - do not edit */
#ifndef HEADARRAY_COMMUNICATIONTHREAD_H_
#define HEADARRAY_COMMUNICATIONTHREAD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void HeadArray_CommunicationThread_entry(void);
#else
extern void HeadArray_CommunicationThread_entry(void);
#endif
#ifdef __cplusplus
extern "C"
{
#endif
extern TX_QUEUE q_HeadArrayCommunicationQueue;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* HEADARRAY_COMMUNICATIONTHREAD_H_ */
