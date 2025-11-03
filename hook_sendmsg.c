#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

static ssize_t (*real_sendmsg)(int sockfd, const struct msghdr *msg, int flags);
static FILE *log_file;

__attribute__((constructor))
static void init(void) {
    real_sendmsg = dlsym(RTLD_NEXT, "sendmsg");
    const char *path = getenv("ALG_PLAINTEXT_LOG");
    if (!path) path = "alg_plaintext.bin";
    log_file = fopen(path, "wb");
    if (!log_file) {
        perror("fopen log");
    }
}

__attribute__((destructor))
static void fini(void) {
    if (log_file) fclose(log_file);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    if (!real_sendmsg) {
        real_sendmsg = dlsym(RTLD_NEXT, "sendmsg");
    }

    if (log_file && msg && msg->msg_iovlen > 0) {
        struct sockaddr_alg salg;
        socklen_t len = sizeof(salg);
        if (getsockname(sockfd, (struct sockaddr *)&salg, &len) == 0) {
            if (len >= sizeof(salg) && salg.salg_family == AF_ALG) {
                for (size_t i = 0; i < (size_t)msg->msg_iovlen; ++i) {
                    const struct iovec *iov = &msg->msg_iov[i];
                    if (iov->iov_base && iov->iov_len) {
                        fwrite(iov->iov_base, 1, iov->iov_len, log_file);
                    }
                }
                fflush(log_file);
            }
        }
    }

    return real_sendmsg(sockfd, msg, flags);
}
