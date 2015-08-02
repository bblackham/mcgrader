#include "shm_interface.h"
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int last_arg1 = -1;
int wait_for_message(struct shm *shm)
{
    int arg0, arg1;
    shm_wait_for_message(shm, &arg0, &arg1);
    if (arg0 == 1) {
        shm_send_message(shm, arg0, arg1+1);
        assert(arg1 == last_arg1+1);
        last_arg1 = arg1;
    }
    return (arg0 >= 0); // -ve messages exit.
}

int main()
{
    struct shm *shm = open_shm(true);
    shm_send_message(shm, 0xabcd, 0x1234);
    pid_t pid;
    switch ((pid = fork())) {
        case -1: /* error */
            assert(0);
            break;
        case 0: /* child */
        {
            char *argv[2] = {
                "./solution",
                NULL,
            };
            execv(argv[0], argv);
            assert(0);
            break;
        }
        default: /* parent */
            break;
    }

    while (wait_for_message(shm));
    printf("Remote complete\n");
    return 0;
}
