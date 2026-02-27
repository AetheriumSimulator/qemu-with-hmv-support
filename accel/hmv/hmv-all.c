/*
 * QEMU HMV support
 * AI Generated. Testing was not performed due to SELinux restrictions on HarmonyOS.
 * Core HMV accelerator logic.
 */

#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "qemu/accel.h"
#include "accel/accel-ops.h"
#include "accel/accel-cpu-ops.h"
#include "system/memory.h"
#include "system/address-spaces.h"
#include "hmv.h"

HMVState *hmv_state;

static void hmv_set_user_memory(HMVState *s, struct hmv_userspace_memory_region *region)
{
    if (ioctl(s->vmfd, HMV_REGISTER_REGION, region) < 0) {
        error_report("HMV: Failed to register region: %s", strerror(errno));
    }
}

static void hmv_region_add(MemoryListener *listener,
                           MemoryRegionSection *section)
{
    HMVState *s = container_of(listener, HMVState, memory_listener);
    struct hmv_userspace_memory_region region;
    hwaddr start = section->offset_within_address_space;
    ram_addr_t size = int128_get64(section->size);
    void *ram = memory_region_get_ram_ptr(section->mr) + section->offset_within_region;

    if (!memory_region_is_ram(section->mr)) {
        return;
    }

    region.guest_phys_addr = start;
    region.memory_size = size;
    region.userspace_addr = (uintptr_t)ram;
    region.flags = 0; /* Add flags calculation if needed */
    region.slot = 0;  /* Slot management to be added */

    hmv_set_user_memory(s, &region);
}

static void hmv_region_del(MemoryListener *listener,
                           MemoryRegionSection *section)
{
    /* TODO: Implement unregistration if HMV supports it */
}

static MemoryListener hmv_memory_listener = {
    .name = "hmv",
    .region_add = hmv_region_add,
    .region_del = hmv_region_del,
    .priority = MEMORY_LISTENER_PRIORITY_ACCEL,
};

static void hmv_accel_class_init(ObjectClass *oc, const void *data)
{
    AccelClass *ac = ACCEL_CLASS(oc);
    ac->name = "HMV";
    ac->allowed = &hmv_state->allowed;
}

static void hmv_accel_instance_init(Object *obj)
{
    HMVState *s = HMV_ACCEL(obj);

    s->fd = -1;
    s->vmfd = -1;
    s->memory_listener = hmv_memory_listener;
}

static int hmv_init(MachineState *ms)
{
    HMVState *s = HMV_ACCEL(current_accel());
    int ret;

    s->fd = open("/dev/hmv", O_RDWR);
    if (s->fd < 0) {
        error_report("hmv_init: Failed to open /dev/hmv: %s", strerror(errno));
        return -errno;
    }

    s->vmfd = ioctl(s->fd, HMV_NEW_VM, 0);
    if (s->vmfd < 0) {
        error_report("hmv_init: HMV_NEW_VM failed: %s", strerror(errno));
        ret = -errno;
        goto err;
    }

    hmv_state = s;
    s->allowed = true;

    memory_listener_register(&s->memory_listener, &address_space_memory);

    printf("HMV: Accelerator initialized successfully (vmfd=%d)\n", s->vmfd);
    return 0;

err:
    if (s->fd >= 0) {
        close(s->fd);
        s->fd = -1;
    }
    return ret;
}

static void hmv_accel_ops_class_init(ObjectClass *oc, const void *data)
{
    AccelOpsClass *ops = ACCEL_OPS_CLASS(oc);

    ops->create_vcpu_thread = hmv_start_vcpu_thread;
    /* hmv_init_vcpu will be called by QEMU core if we hook it correctly 
     * or we call it manually from create_vcpu_thread. 
     */
}

static const TypeInfo hmv_accel_type = {
    .name = TYPE_HMV_ACCEL,
    .parent = TYPE_ACCEL,
    .instance_init = hmv_accel_instance_init,
    .class_init = hmv_accel_class_init,
    .instance_size = sizeof(HMVState),
};

static const TypeInfo hmv_accel_ops_type = {
    .name = ACCEL_OPS_NAME("hmv"),
    .parent = TYPE_ACCEL_OPS,
    .class_init = hmv_accel_ops_class_init,
    .abstract = true,
};

static void hmv_type_init(void)
{
    type_register_static(&hmv_accel_type);
    type_register_static(&hmv_accel_ops_type);
}

type_init(hmv_type_init);
