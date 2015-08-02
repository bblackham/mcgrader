#ifndef __SHM_INTERFACE_H__
#define __SHM_INTERFACE_H__

/**
 * Defines a shared memory mailbox-like interface between two processes.
 *
 * One process is nominated to be the server, which must be created first.
 * The other process is the client.
 */

#include <stdbool.h>

struct shm;

/**
 * Open a new shared memory interface to a remote process.
 */
struct shm *open_shm(bool server);

/**
 * Close a shared memory interface.
 */
void close_shm(struct shm *shm);

/**
 * Wait for a message from the remote process.
 */
void shm_wait_for_message(struct shm *shm, int *arg0, int *arg1);

/**
 * Read a message from the remote process without blocking. If no message is
 * ready, returns false.
 */
bool shm_wait_for_message_nb(struct shm *shm, int *arg0, int *arg1);

/**
 * Send a message to a remote process. This function does not block. However,
 * if is called before the remote process has received the previous message, it
 * may overwrite the previous message. The caller should ensure that the
 * previous message has been received (e.g. by waiting for a response), before
 * calling this function.
 */
void shm_send_message(struct shm *shm, int arg0, int arg1);

#endif /* __SHM_INTERFACE_H__ */
