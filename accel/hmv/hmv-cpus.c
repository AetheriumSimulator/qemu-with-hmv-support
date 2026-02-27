/*
 * QEMU HMV support - vCPU Management
 * AI Generated. Testing was not performed due to SELinux restrictions on HarmonyOS.
 */

#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qemu/main-loop.h"
#include "qemu/guest-random.h"
#include "system/cpus.h"
#include "system/runstate.h"
#include "hmv.h"

static void *hmv_vcpu_thread_fn(void *arg)
{
    CPUState *cpu = arg;
    int ret;

    rcu_register_thread();
    bql_lock();
    qemu_thread_get_self(cpu->thread);
    cpu->thread_id = qemu_get_thread_id();
    current_cpu = cpu;

    /* TODO: Init vCPU registers here if needed */

    qemu_guest_random_seed_thread_part2(cpu->random_seed);

    /* Signal that the vCPU is ready */
    cpu_thread_signal_created(cpu);

    do {
        qemu_process_cpu_events(cpu);

        if (cpu_can_run(cpu)) {
            bql_unlock();

            /* Start Guest Execution */
            ret = ioctl(hmv_vcpu_fd(cpu), HYP_VCPU_START, 0);

            bql_lock();

            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                }
                error_report("HMV: HYP_VCPU_START failed: %s", strerror(errno));
                break;
            }

            /* Handle Exit (ret == 0 or based on shared exit info) */
            /*
             * Based on the archive:
             * Read __vmexit_info, handle MMIO, etc.
             * If Activation Pool is used, some Exits might be handled asynchronously.
             */
        }
        /* Handled by qemu_process_cpu_events at loop start */
    } while (!cpu->unplug || cpu_can_run(cpu));

    cpu_thread_signal_destroyed(cpu);
    bql_unlock();
    rcu_unregister_thread();
    return NULL;
}

void hmv_start_vcpu_thread(CPUState *cpu)
{
    char thread_name[VCPU_THREAD_NAME_SIZE];

    snprintf(thread_name, VCPU_THREAD_NAME_SIZE, "CPU %d/HMV", cpu->cpu_index);
    qemu_thread_create(cpu->thread, thread_name, hmv_vcpu_thread_fn,
                       cpu, QEMU_THREAD_JOINABLE);
}

int hmv_init_vcpu(CPUState *cpu)
{
    HMVState *s = hmv_state;
    int vcpu_fd;

    vcpu_fd = ioctl(s->vmfd, HMV_NEW_VCPU, cpu->cpu_index);
    if (vcpu_fd < 0) {
        error_report("HMV: HMV_NEW_VCPU failed for CPU %d: %s",
                     cpu->cpu_index, strerror(errno));
        return -errno;
    }

    /* Store the vCPU fd via the opaque pointer */
    hmv_set_vcpu_fd(cpu, vcpu_fd);

    return 0;
}
