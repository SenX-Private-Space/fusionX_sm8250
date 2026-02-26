#ifndef _LINUX_FUSIONX_ATTRIBUTES_H
#define _LINUX_FUSIONX_ATTRIBUTES_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#include <linux/sched.h>

#define FUSIONX_BLOCKLIST_STRLEN 256
#define FUSIONX_MAX_BLOCKED 16

bool fusionx_comm_blocked(const char *comm);

struct fusionx_attributes {
    int kgsl_skip_zeroing;
    int avoid_dirty_pte;
    char bg_blocklist[FUSIONX_BLOCKLIST_STRLEN];
};

extern struct fusionx_attributes fusionx_data;

#endif /* _LINUX_FUSIONX_ATTRIBUTES_H */
