#pragma once

#include <stdio.h>

#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char __StackLimit;
extern char __bss_end__;
extern char* sbrk(int i);

static inline uint32_t getTotalHeap(void) {
    return (uint32_t)(&__StackLimit - &__bss_end__);
}

static inline uint32_t getFreeHeap(void) {
    char* heap_end = sbrk(0);
    return (uint32_t)(&__StackLimit - heap_end);
}

static inline uint32_t getUsedHeap(void) {
    return getTotalHeap() - getFreeHeap();
}

static inline void print_ram_usage(void) {
    uint32_t total_heap = getTotalHeap();
    uint32_t free_heap = getFreeHeap();
    uint32_t used_heap = getUsedHeap();

    extern char __data_start__;
    uint32_t static_used = (uint32_t)(&__bss_end__ - &__data_start__);

    printf("---- RAM USAGE ----\n");
    printf("Static (.data+.bss) : %lu bytes\n", static_used);
    printf("Heap total          : %lu bytes\n", total_heap);
    printf("Heap used           : %lu bytes\n", used_heap);
    printf("Heap free           : %lu bytes\n", free_heap);
    printf("--------------------\n");
}

static inline packet_tx_t handle_system_memory(packet_rx_t rx) {
    print_ram_usage();
    return (packet_tx_t){.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd};
}

#ifdef __cplusplus
}
#endif