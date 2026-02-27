/*
 * QEMU HMV support - UAPI Definitions
 * AI Generated. Testing was not performed due to SELinux restrictions on HarmonyOS.
 * Derived from HMV for QEMU Integration Documentation.
 */

#ifndef QEMU_HMV_H
#define QEMU_HMV_H

#include "qemu/osdep.h"
#include <sys/ioctl.h>
#include "hw/core/cpu.h"
#include "qemu/accel.h"
#include "accel/accel-ops.h"
#include "accel/accel-cpu-ops.h"
#include "system/cpus.h"
#include "system/memory.h"

/* HMV IOCTLs - Using placeholder magic numbers based on typical patterns */
#define HMV_TYPE          0xAE

#define HMV_NEW_VM        _IO(HMV_TYPE,   0x01)
#define HMV_NEW_VCPU      _IOW(HMV_TYPE,  0x02, uint32_t)
#define HMV_REGISTER_REGION _IOW(HMV_TYPE, 0x03, struct hmv_userspace_memory_region)

/* vCPU Control */
#define HYP_VCPU_START    _IO(HMV_TYPE,   0x10)
#define HYP_VCPU_SETREG   _IOW(HMV_TYPE,  0x11, struct hmv_regs)
#define HYP_VCPU_GETREG   _IOWR(HMV_TYPE, 0x12, struct hmv_regs)
#define HYP_VCPU_SET_CTX  _IOW(HMV_TYPE,  0x13, struct hmv_cpu_context)

/* IRQ and Capability */
#define HM_VM_ASSERT_IRQ  _IOW(HMV_TYPE,  0x20, uint32_t)
#define HM_SYSCAP_BIND_DISPATCHER _IOW(HMV_TYPE, 0x30, struct hmv_dispatcher_bind)

/* VM Exit Return Codes */
#define __ACTV_WFERET_HYP_VMEXIT_OK    0
#define __ACTV_WFERET_HYP_VMEXIT_CRASH 1

struct hmv_userspace_memory_region {
    uint32_t slot;
    uint32_t flags;
    uint64_t guest_phys_addr;
    uint64_t memory_size;
    uint64_t userspace_addr;
};

/* Placeholder structures for registers - to be refined based on ARM64 */
struct hmv_regs {
    uint64_t regs[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
};

struct hmv_cpu_context {
    /* Context data for HYP_VCPU_SET_CTX */
    uint64_t data;
};

struct hmv_dispatcher_bind {
    uint64_t gpa_start;
    uint64_t size;
    /* Activation pool info */
};

typedef struct HMVState {
    AccelState parent_obj;
    int fd;
    int vmfd;
    bool allowed;
    MemoryListener memory_listener;
} HMVState;

#define TYPE_HMV_ACCEL "hmv-accel"
#define HMV_ACCEL(obj) \
    OBJECT_CHECK(HMVState, (obj), TYPE_HMV_ACCEL)

/* Global state, declared in hmv-all.c */
extern HMVState *hmv_state;

/* Prototypes */
void hmv_start_vcpu_thread(CPUState *cpu);
int hmv_init_vcpu(CPUState *cpu);

/*
 * Store the HMV vCPU fd in cpu->opaque.
 * This avoids modifying the core CPUState structure.
 */
static inline int hmv_vcpu_fd(CPUState *cpu)
{
    return (int)(intptr_t)cpu->opaque;
}

static inline void hmv_set_vcpu_fd(CPUState *cpu, int fd)
{
    cpu->opaque = (void *)(intptr_t)fd;
}

#endif /* QEMU_HMV_H */
