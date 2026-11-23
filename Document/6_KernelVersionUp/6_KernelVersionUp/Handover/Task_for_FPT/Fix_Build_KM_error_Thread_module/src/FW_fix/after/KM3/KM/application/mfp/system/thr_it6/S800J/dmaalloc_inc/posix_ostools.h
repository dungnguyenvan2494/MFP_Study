/*
**************************************************************************
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this file,
You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2012-2014, Marvell International Ltd.

Alternatively, this software may be distributed under the terms of the GNU
General Public License Version 2, and any use shall comply with the terms and
conditions of the GPL.  A copy of the GPL is available at
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
ARE EXPRESSLY DISCLAIMED.  The GPL license provides additional details about
this warranty disclaimer.
******************************************************************************
*/


/**
 * \file posix_ostools.h
 *
 * \brief Common posix helper functions and definitions to help
 *        with translation from threadx code to posix code.
 * 
 * \note Note that all of the timeouts of the underlying posix APIs
 * are based on CLOCK_REALTIME.  Since posix specifies that the timeouts
 * are absolute this means that they are subject to firing if the system
 * clock is updated.  This can happen either because a user interacts
 * with the system and updates the time or because an automated process
 * such as ntpd updates the clock.
 **/

#ifndef POSIX_OSTOOLS_H
#define POSIX_OSTOOLS_H

#ifndef CLOCK_MONOTONIC
// define CLOCK_MONOTONIC same as CLOCK_REALTIME if not already defined.
#define CLOCK_MONOTONIC CLOCK_REALTIME
#endif

/** 
 *  \brief Common time definitions
 **/ 
#define USEC_PER_SECOND         1000000  /* one million */ 
#define USEC_PER_MILLISEC       1000     /* one thousand */
#define NANOSEC_PER_SECOND      1000000000 /* one BILLLION */
#define NANOSEC_PER_USEC        1000     /* one thousand */
#define NANOSEC_PER_MILLISEC    1000000  /* one million */
#define MILLISEC_PER_TICK       10
#define MILLISEC_PER_SECOND     1000

#define POSIX_WAIT_FOREVER (-1)

/** 
 *  \brief The default priority on calls to mq_send() and
 *         mq_receive()
 **/
#define MQ_DEFAULT_PRIORITY  0

/** 
 *  \brief The system thread scheduling policy
 **/
#define POSIX_DEFAULT_SCHED_POLICY (SCHED_RR)

/** 
 *  \brief The thread priority defines.  When the SDK is built for Linux
 *  all threads with NORMAL priority and below are run using the standard linux
 *  scheduling algorithm (SCHED_OTHER).  Threads with a priority of REALTIME and
 *  higher are run as realtime threads with a SCHED_RR policy.
 **/
#define POSIX_THR_PRI_HIGHEST       (sched_get_priority_max(POSIX_DEFAULT_SCHED_POLICY))
#define POSIX_THR_PRI_LOWEST        (sched_get_priority_min(POSIX_DEFAULT_SCHED_POLICY))
#define POSIX_THR_PRI_NORMAL        ((POSIX_THR_PRI_HIGHEST + POSIX_THR_PRI_LOWEST)/2)
#define POSIX_THR_PRI_ISR           (POSIX_THR_PRI_NORMAL + 10)
#define POSIX_THR_PRI_IMAGE         (POSIX_THR_PRI_NORMAL + 3)
#define POSIX_THR_PRI_DRIVER        (POSIX_THR_PRI_NORMAL + 2)
#define POSIX_THR_PRI_ENGINE        (POSIX_THR_PRI_NORMAL + 1)
#define POSIX_THR_PRI_REALTIME      (POSIX_THR_PRI_NORMAL + 1)
#define POSIX_THR_PRI_LOW           (POSIX_THR_PRI_NORMAL - 5)
#define POSIX_THR_PRI_WIRELESS      (POSIX_THR_PRI_NORMAL - 10)
#define POSIX_THR_PRI_SERIAL_IN     (POSIX_THR_PRI_LOWEST + 1)
#define POSIX_THR_PRI_SERIAL_OUT    (POSIX_THR_PRI_LOWEST)

/** 
 * \brief  Minimum thread stack sizes  
 **/ 
#ifdef __linux__
#include <limits.h>
#define POSIX_MIN_STACK_SIZE PTHREAD_STACK_MIN
#else
#define POSIX_MIN_STACK_SIZE (8*1024)
#endif

#ifdef __linux__
# include <pthread.h>
# ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#  define POSIX_MUTEX_INITIALIZER  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
# else
#  define POSIX_MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
# endif
#else
# define POSIX_MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
#endif

/** 
 * \brief  Helper to deal with difference between linux and
 * threadx errno  
**/ 
#ifdef __linux__
#include <unistd.h>  // sleep
#include <errno.h>
#define thread_errno errno
#else
#define thread_errno posix_get_pthread_errno(pthread_self())
#endif

#include <stdint.h>
#include <pthread.h>
#ifdef __linux__
#include <mqueue.h>
#include <semaphore.h>
#endif

#define posix_wait_for_MESSAGE( msgq, msg, timeout_in_usec ) \
    posix_wait_for_message( msgq, (char*)msg, sizeof(MESSAGE), timeout_in_usec) 

#define posix_wait_for_sys_message( msgq, msg, timeout_in_ticks ) \
    posix_wait_for_message( msgq, (char*)msg, sizeof(MESSAGE), \
        (timeout_in_ticks == POSIX_WAIT_FOREVER) ? POSIX_WAIT_FOREVER : ((timeout_in_ticks)*(MILLISEC_PER_TICK*USEC_PER_MILLISEC)))

/**
 * \brief  Wait a specific number of microseconds for a message
 *
 * Convenience wrapper function around mq_timedreceive()
 *
 * \retval integer ; the errno from the mq_timedreceive() call
 *
 */
int posix_wait_for_message( mqd_t msgq, char *msg, int msg_size, 
                            unsigned long int timeout_in_usec );

/**
 * \brief  Send a message waiting a specific number of 
 *         microseconds.
 *
 * Convenience wrapper function around mq_timedsend()
 *
 * \retval integer ; the errno from the mq_timedsend() call
 */
int posix_message_send( mqd_t msgq, char *msg, int msg_size, unsigned long int priority, unsigned long int timeout_in_usec );

#define posix_send_sys_message(msgq, msg, timeout_in_ticks) \
    posix_message_send( (msgq), (msg), sizeof(MESSAGE), MQ_DEFAULT_PRIORITY, \
        (timeout_in_ticks == POSIX_WAIT_FOREVER) ? POSIX_WAIT_FOREVER : ((timeout_in_ticks)*(MILLISEC_PER_TICK*USEC_PER_MILLISEC)))

/**
 * \brief Creates a named message queue and establishes a 
 *        connection between it and the calling thread
 *  
 * \param msgq_ptr      Message Queue pointer of type mqd_t 
 * \param queue_name    Name of message queue to open 
 * \param num_messages  Maximum number of messages 
 */
#define posix_create_MESSAGE_queue( msgq_ptr, queue_name, num_messages ) \
    posix_create_message_queue( msgq_ptr, queue_name, num_messages, sizeof(MESSAGE) )

/**
 * \brief Creates a named message queue and establishes a 
 *        connection between it and the calling thread
 *  
 * \param msgq_ptr      Message Queue pointer of type mqd_t 
 * \param queue_name    Name of message queue to open 
 * \param num_messages  Maximum number of messages 
 * \param message_size  Size of the messages 
 */
void posix_create_message_queue( mqd_t *msgq_ptr, const char *queue_name, 
                                 int num_messages, int message_size );

/** 
 *  \brief add a 64bit long usec value to the current time for 
 *         use in creating an absolute timeout
 */
void posix_calc_future_usec( struct timespec *ts, 
                             unsigned long int timeout_usec );
                             
 /**
 * \brief  subtract two timespec values
 *
 *  Not a POSIX function!
 *
 * returns <0 if a<b
 *          0 if a==b
 *         >0 if a>b
 *
 * If a<b, the values in the timespec will be subject to integer underflow.
 *
 */                            
int posix_timespec_sub( const struct timespec *a,
                        const struct timespec *b,
                        struct timespec *result );

/**
 * \brief Get the number of items currently in a message queue waiting to be pulled out.
 * 
 * Originally created to help track memory leaks (messages can contain pointers
 * to malloc'd memory)
 *
 * \param msgq  Message queue to check for items in 
 *  
 * \retval      The number of messages pending in the queue 
 *  
 **/ 
long int posix_get_queue_pending( mqd_t msgq );

/**
 * \brief  Create a thread, with optional stack and priority.
 *
 * Wrapper around the pthread calls to create a thread assigned to a specific
 * stack.
 *  
 * \param thread_id       Newly created thread ID of type thread_t
 * \param start_routine   Thread startup routine 
 * \param thread_arg      Arguments to startup routine by ref
 * \param thread_name     Thread Name 
 * \param stack_ptr       Pointer to Address of the stack 
 * \param stack_size      Size of the stack 
 *  
 * \retval              0 if successful  
 */
int posix_create_thread(pthread_t *thread_id, 
                        void *(*start_routine)(void*), 
                        void *thread_arg, 
                        const char* thread_name, 
                        void *stack_ptr, 
                        size_t stack_size, int priority);

/**
 * \brief Sleep for specified number of ticks.  This is a 
 *        convenience wrapper around nanosleep.
 */
#define posix_sleep_ticks(_ticks_) \
    posix_sleep_ms(_ticks_*MILLISEC_PER_TICK)

/**
 * \brief Sleep for specified number of milliseconds.  This is a 
 *        convenience wrapper around nanosleep.
 */
int posix_sleep_ms(uint32_t timeout_ms);

/**
 * \brief Sleep for specified number of microseconds.  This is a 
 *        convenience wrapper around nanosleep.
 */
int posix_sleep_us(uint32_t timeout_us);

/**
 * \brief Get system relative time in ticks.  Based on CLOCK_MONOTONIC. 
 * \note Since this is based on CLOCK_MONOTONIC it is suitable only for debug 
 * and relative timing calculation purposes.  The time returned by this routine 
 * CANNOT be used to calculation absoluted timeout values for any of the posix 
 * interfaces that require absolute timeout values such as mq_timedsend, 
 * sem_timedwait, etc.
 */
#define posix_gettime_ticks() (posix_gettime_ms()/MILLISEC_PER_TICK)

/**
 * \brief Get system relative time in milliseconds.  Based on CLOCK_MONOTONIC. 
 * \note Since this is based on CLOCK_MONOTONIC it is suitable only for debug 
 * and relative timing calculation purposes.  The time returned by this routine 
 * CANNOT be used to calculation absoluted timeout values for any of the posix 
 * interfaces that require absolute timeout values such as mq_timedsend, 
 * sem_timedwait, etc. 
 */
uint32_t posix_gettime_ms( void );

/**
 * \brief Initialize the pthread mutex object referenced by 
 *        mutex.  Upon successful the mutex state is initialized
 *        and unlocked.
 *  
 * \param mutex   Pointer to a pthread mutex object 
 * \retval        0 if successful 
 */
int posix_mutex_init(pthread_mutex_t *mutex);

/**
 * \brief Create and open a named semaphore with an initial value. 
 * Unlink is invoked on the name prior to creation so any existing 
 * semaphore with the same name is removed from the namespace and 
 * potentially destroyed. 
 *  
 * \param name  Name of the semaphore as a NULL terminated string.
 * \param init_value    Initial value of the semaphore
 * \retval      Pointer to semaphore if successful; else NULL with errno 
 * set to the error. 
 */
sem_t* posix_sem_open(const char* name, uint32_t init_value);

/**
 * \brief Lock a semaphore with a timeout specified in milliseconds.
 *  
 * \param sem   Pointer to an initialized semaphore object .
 * \param timeout_ms Milliseconds to wait before timing out.
 * \retval        0 if successful; -ETIMEDOUT on timeout; else -errno
 */
int posix_sem_timedwait_ms(sem_t *sem, uint32_t timeout_ms);


pid_t gettid( void ) noexcept;	//  24/03/22 FPT	Fix build error GccVersionUp.
void posix_display_thread_sched_attr(const char* name);

#endif // eof
