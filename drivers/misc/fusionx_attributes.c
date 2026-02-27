#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/fusionx_attributes.h>

static char fusionx_blocked[FUSIONX_MAX_BLOCKED][TASK_COMM_LEN];
static u8 fusionx_blocked_len[FUSIONX_MAX_BLOCKED];
static int fusionx_blocked_cnt;

struct fusionx_attributes fusionx_data = {
	.kgsl_skip_zeroing = 0,
	.avoid_dirty_pte = 0,
	.bg_blocklist = "com.shopee.id,com.lazada.android,com.tokopedia.tkpd",
};

bool fusionx_comm_blocked(const char *comm)
{
	int i;

	for (i = 0; i < fusionx_blocked_cnt; i++) {
		if (!strncmp(comm, fusionx_blocked[i], fusionx_blocked_len[i]))
			return true;
	}

	return false;
}
EXPORT_SYMBOL_GPL(fusionx_comm_blocked);

static void fusionx_rebuild_blocklist(char *buf)
{
	char *p = buf;
	char *token;

	fusionx_blocked_cnt = 0;
	while ((token = strsep(&p, ",")) &&
	       fusionx_blocked_cnt < FUSIONX_MAX_BLOCKED) {
		if (!*token)
			continue;

		strscpy(fusionx_blocked[fusionx_blocked_cnt], token, TASK_COMM_LEN);

		fusionx_blocked_len[fusionx_blocked_cnt] =
			strlen(fusionx_blocked[fusionx_blocked_cnt]);

		pr_alert("FUSIONX: blocking '%s'\n",
			 fusionx_blocked[fusionx_blocked_cnt]);
		fusionx_blocked_cnt++;
	}
	pr_alert("FUSIONX: total blocked apps = %d\n", fusionx_blocked_cnt);
}

static ssize_t bg_blocklist_show(struct kobject *kobj,
				 struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", fusionx_data.bg_blocklist);
}

static ssize_t bg_blocklist_store(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	char tmp[FUSIONX_BLOCKLIST_STRLEN];

	strscpy(tmp, buf, sizeof(tmp));
	strreplace(tmp, '\n', '\0');
	strscpy(fusionx_data.bg_blocklist, tmp, sizeof(fusionx_data.bg_blocklist));

	fusionx_rebuild_blocklist(tmp);

	return count;
}

static struct kobj_attribute bg_blocklist_attr =
	__ATTR(bg_blocklist, 0664, bg_blocklist_show, bg_blocklist_store);

#define FUSIONX_ATTR_RW(name)                                                     \
	static ssize_t name##_show(struct kobject *kobj,                       \
				   struct kobj_attribute *attr, char *buf)     \
	{                                                                      \
		return sprintf(buf, "%d\n", fusionx_data.name);                   \
	}                                                                      \
	static ssize_t name##_store(struct kobject *kobj,                      \
				    struct kobj_attribute *attr,               \
				    const char *buf, size_t count)             \
	{                                                                      \
		int val;                                                       \
		if (kstrtoint(buf, 10, &val))                                  \
			return -EINVAL;                                        \
		fusionx_data.name = val;                                          \
		return count;                                                  \
	}                                                                      \
	static struct kobj_attribute name##_attr =                             \
		__ATTR(name, 0644, name##_show, name##_store);

FUSIONX_ATTR_RW(kgsl_skip_zeroing);
FUSIONX_ATTR_RW(avoid_dirty_pte);

static struct attribute *fusionx_attrs[] = { &kgsl_skip_zeroing_attr.attr,
					  &avoid_dirty_pte_attr.attr,
					  &bg_blocklist_attr.attr, NULL };

static struct attribute_group fusionx_attr_group = {
	.attrs = fusionx_attrs,
};

static struct kobject *fusionx_kobj;

static int __init fusionx_attributes_init(void)
{
	int retval;
	char tmp[FUSIONX_BLOCKLIST_STRLEN];

	if (fusionx_data.bg_blocklist[0]) {
		strscpy(tmp, fusionx_data.bg_blocklist, sizeof(tmp));
		fusionx_rebuild_blocklist(tmp);
	}

	fusionx_kobj = kobject_create_and_add("fusionx_attributes", kernel_kobj);
	if (!fusionx_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(fusionx_kobj, &fusionx_attr_group);
	if (retval)
		kobject_put(fusionx_kobj);

	return retval;
}

static void __exit fusionx_attributes_exit(void)
{
	kobject_put(fusionx_kobj);
}

module_init(fusionx_attributes_init);
module_exit(fusionx_attributes_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kvsnr113 <kvsnrprojkt113@gmail.com>");
MODULE_DESCRIPTION("FUSIONX kgsl skip zeroing attribute module");
