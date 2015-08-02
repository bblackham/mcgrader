#include "shm_interface.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#if 1 // These are only on 64-bit x86. 32-bit x86 will need something else.
    #define rmb() asm volatile("lfence" ::: "memory")
    #define wmb() asm volatile("sfence" ::: "memory")
#else
#error
#endif

#define SHM_FILE "/tmp/grader.shm"

#ifndef CACHE_LINE_SIZE
#error "Build file must specify CACHE_LINE_SIZE"
#endif
#define ALIGN(x) __attribute__((aligned(x)))

struct shm_mbox {
    ALIGN(CACHE_LINE_SIZE)
    int ready;
    ALIGN(CACHE_LINE_SIZE)
    uint32_t arg0;
    uint32_t arg1;
};

struct shm_page {
    struct shm_mbox s2c;
    struct shm_mbox c2s;
};

struct shm {
    bool am_server;
    int fd;
    struct shm_page *shm_page;
};

struct shm *open_shm(bool server)
{
    // Server must open first.
    int fd = open(SHM_FILE, O_RDWR | (server ? O_CREAT|O_TRUNC : 0), 0600);
    assert(fd >= 0);
    int ret = ftruncate(fd, sizeof(struct shm_page));
    assert(ret == 0);

    struct shm_page *p = mmap(NULL, sizeof(*p), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert (p != MAP_FAILED);

    struct shm *shm = calloc(1, sizeof(*shm));
    assert(shm != NULL);

    shm->am_server = server;
    shm->fd = fd;
    shm->shm_page = p;

    if (server) {
        memset(p, 0, sizeof(*p));
    }

    return shm;
}

void close_shm(struct shm *shm)
{
    unlink(SHM_FILE);
    close(shm->fd);
    munmap(shm->shm_page, sizeof(struct shm_page));
    free(shm);
}

bool shm_wait_for_message_nb(struct shm *shm, int *arg0, int *arg1)
{
    struct shm_mbox *mbox;
    if (shm->am_server)
        mbox = &shm->shm_page->c2s;
    else
        mbox = &shm->shm_page->s2c;

    asm volatile ("" : : : "memory");
    if (mbox->ready == 0)
        return false;

    // read barrier to ensure all future reads are issued after the above read
    // of ready.
    rmb();

    *arg0 = mbox->arg0;
    *arg1 = mbox->arg1;

    mbox->ready = 0;
    // Ensure above write is drained to memory.
    wmb();
    return true;
}

void shm_wait_for_message(struct shm *shm, int *arg0, int *arg1)
{
    struct shm_mbox *mbox;
    if (shm->am_server)
        mbox = &shm->shm_page->c2s;
    else
        mbox = &shm->shm_page->s2c;
    while (mbox->ready == 0) {
        rmb();
        asm volatile ("" : : : "memory");
    }
    // read barrier to ensure all future reads are issued after
    // the above read of ready.
    rmb();

    *arg0 = mbox->arg0;
    *arg1 = mbox->arg1;

    mbox->ready = 0;
    wmb();
}

void shm_send_message(struct shm *shm, int arg0, int arg1)
{
    struct shm_mbox *mbox;
    if (shm->am_server)
        mbox = &shm->shm_page->s2c;
    else
        mbox = &shm->shm_page->c2s;

    mbox->arg0 = arg0;
    mbox->arg1 = arg1;
    // write barrier to ensure all past writes have been made globally visible.
    wmb();
    mbox->ready = 1;
    // write barrier to force drain of write buffer.
    wmb();
}
