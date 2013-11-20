#include <stdlib.h>
#include <string.h>
#include "client_queue.h"
/** @file
	@brief Client's messages queue management functions

	This file provides a module that knows how to operate on a client's messages queue.
	It is easy for a client's thread to wake up and read incoming data using an IO watcher. However, sporadically, we also need to wake up a client's thread to write to his socket.
	For example, if user A PRIVMSGs user B, user A's thread must be able to somehow inform user B thread that something needs to be sent to user B.
	To do so, we use an async watcher. An async watcher allows an arbitrary thread X to wake up another thread Y. Thread Y must be running an events loop and must have initialized and started an async watcher.
	An async watcher works pretty much the same way as an IO watcher, but libev's documentation explicitly states that queueing is not supported. In other words, if more async messages arrive when we are processing
	an async callback, these will be silently discarded. To avoid losing messages like this, we implement our own messages queueing system.
	Each client holds a queue of messages waiting to be written to his socket. These messages can originate from any thread.
	
	Every operation in a client's queue shall be invoked through the use of the functions declared in this file.
	
	These functions are not thread-safe. It is assumed that they are called by a thread-safe module.
	
	@author Filipe Goncalves
	@date November 2013
*/

/** Initializes a queue. This function is typically called when a new client is created.
	No queue insertions or deletions can be performed before initializing a queue.
	@param queue The queue to initialize.
	@return `0` on success; `-1` if there aren't enough resources to initialize a queue.
	@warning Undefined behavior will occur if queue operations are invoked in a non-initialized queue.
*/
int client_queue_init(struct msg_queue *queue) {
	queue->top = 0;
	queue->bottom = 0;
	queue->elements = 0;
	return pthread_mutex_init(&queue->mutex, NULL);
}

/** Destroys a queue. This function is typically called when a client is exiting and is about to be destroyed.
	@param queue The queue to destroy.
	@return `0` on success, `-1` if the destroy operation was not successfull. The error condition should never happen if this module is used properly and no race conditions occur.
			If `-1` is returned, it means something went terribly wrong with the thread synchronization mechanism used in a queue. For example, maybe a queue destroy operation was invoked when
			a lock to this queue was active.
	@warning Queue destroy operations must be performed carefully to avoid race conditions. The code that uses this module must ensure that after a queue is destroyed,
			 no other thread will try to insert new messages in this queue. The program shall consider the case that a thread successfully destroys a queue, and is immediately
			 interrupted after that, switching to another thread that was stopped precisely in an enqueue operation. This situation cannot occur; the IRCd will fail miserably and crash
			 if it happens.
*/
int client_queue_destroy(struct msg_queue *queue) {
	int i;
	int j;
	for (i = queue->bottom, j = 0; j < queue->elements; i = (i+1)%WRITE_QUEUE_SIZE, j++) {
		free(queue->messages[i]);
	}
	return pthread_mutex_destroy(&queue->mutex);
}

/** Inserts a new message in a queue.
	@param queue The target queue where the message shall be written to.
	@param message A null terminated characters sequence to enqueue. A fresh new copy of `message` is performed using `strdup()`, to ensure that the characters sequence lives for as long as it is needed. 
				   The caller of this function need not worry about allocating and freeing resources, this module will take care of that.
	@return `0` on success; `-1` if there is no space left in this client's queue, or if there's no memory to perform a fresh copy of the message.
*/
int client_enqueue(struct msg_queue *queue, char *message) {
	char *str;
	pthread_mutex_lock(&queue->mutex);
	if ((str = strdup(message)) == NULL || queue->elements == WRITE_QUEUE_SIZE) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}
	queue->messages[queue->top] = str;
	queue->top = (queue->top+1)%WRITE_QUEUE_SIZE;
	queue->elements++;
	pthread_mutex_unlock(&queue->mutex);
	return 0;
}

/** Dequeues a message previously enqueued. Dequeue operations follow a FIFO policy.
	@param queue The target queue to extract the message.
	@return Pointer to a null terminated characters sequence previously inserted in this queue; `NULL` if there are no elements. The pointer returned must be freed by the caller after it is no longer needed.
	@warning A memory leak will occur if the caller does not free the pointer returned when it no longer needs it.
*/
char *client_dequeue(struct msg_queue *queue) {
	char *ptr;
	pthread_mutex_lock(&queue->mutex);
	if (queue->elements == 0) {
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}
	ptr = queue->messages[queue->bottom];
	queue->bottom = (queue->bottom+1)%WRITE_QUEUE_SIZE;
	queue->elements--;
	pthread_mutex_unlock(&queue->mutex);
	return ptr;
}

/** Determines if a queue is empty.
	@param queue The queue to examine.
	@return `0` if the queue is not empty; `1` if the queue is empty.
*/
inline int client_is_queue_empty(struct msg_queue *queue) {
	int ret;
	pthread_mutex_lock(&queue->mutex);
	ret = (queue->elements == 0);
	pthread_mutex_unlock(&queue->mutex);
	return ret;
}

/** Atomically iterates destructively through a queue, calling the specified function for each element that is dequeued.
	@param queue The queue to iterate.
	@param f A pointer to a function that shall be called for each element dequeued. This function is passed a pointer to the message dequeued as its first parameter,
			 and is passed `args` as its second parameter.
	@param args A generic pointer to an arbitrary argument that will be passed as the second parameter when `(*f)()` is called.
	@warning This function dequeues every message. When this function returns, the queue will be empty.
	@warning The function pointed to by `f` shall not invoke actions that can possibly result in `pthread_exit()` being called. The function is invoked while a lock to the queue is on hold;
			 calling `pthread_exit()` inside `f` will cause undefined behavior.
	@warning The function pointed to by `f` must free the message as long as it is not needed anymore. Memory leaks will occur if it doesn't do so.
*/
void client_queue_foreach(struct msg_queue *queue, void (*f)(char *, void *), void *args) {
	int i;
	int j;
	pthread_mutex_lock(&queue->mutex);
	for (i = queue->bottom, j = 0; j < queue->elements; j++, i = (i+1)%WRITE_QUEUE_SIZE) {
		(*f)(queue->messages[i], args);
	}
	queue->bottom = queue->top = queue->elements = 0;
	pthread_mutex_unlock(&queue->mutex);
}