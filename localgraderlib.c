#include "shm_interface.h"
#include <assert.h>
#include <stdio.h>

#define LIM (1000)

int main()
{
    struct shm *shm = open_shm(false);

    int arg0, arg1;
    // We expect the remote grader to have already left us a message so that we
    // can start instantly.
    bool success = shm_wait_for_message_nb(shm, &arg0, &arg1);
    assert(success);
    assert(arg0 == 0xabcd);
    assert(arg1 == 0x1234);

    // Iterate from 0 .. LIM sending messages back and forth.
    for (int i = 0; i < LIM; i++) {
        shm_send_message(shm, 1, i);
        shm_wait_for_message(shm, &arg0, &arg1);
        assert(arg0 == 1);
        assert(arg1 == i+1);
    }

    printf("Counted to %d\n", LIM);

    // signal exit to remote grader.
    shm_send_message(shm, -1, -1);

    return 0;
}
