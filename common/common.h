#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#define ACCESS_REG(r) (*(volatile uint64_t *) ((uintptr_t) regs + r))

#define DEBUG
//#define DEBUG_MMIO Whether to display MMIO information

inline static char* get_hostname() {
    static char hostname[256] = "unknown"; 
    static int is_initialized = 0;   
    if (!is_initialized) {
        gethostname(hostname, sizeof(hostname));
        is_initialized = 1; // 标记为已初始化
    }
    return hostname; // 返回存储的主机名
}

#ifdef DEBUG
#define eprintf(fmt, ...) \
    fprintf(stderr, "ꚰꚰꚰ %s:%s:%d %s:%d-%luꚰꚰꚰ : " fmt "\n", __FILE__, __func__, __LINE__,get_hostname(),getpid(),pthread_self(), ##__VA_ARGS__)

#define dprintf(fmt, ...) \
    fprintf(stdout, "ꚰꚰꚰ %s:%s:%d %s:%d-%luꚰꚰꚰ : " fmt "\n", __FILE__, __func__, __LINE__,get_hostname(),getpid(),pthread_self(), ##__VA_ARGS__)
#else
#define eprintf(...) do { } while (0)
#define dprintf(...) do { } while (0)
#endif

#endif
