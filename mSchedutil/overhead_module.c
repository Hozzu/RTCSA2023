#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>

MODULE_AUTHOR("Philkyue Shin <pkshin@redwood.snu.ac.kr>");
MODULE_DESCRIPTION("A module that enables overhead tracking");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

extern u64 total_mSchedutil_time;
extern u64 total_governor_time;

extern int pkshin_overhead_enabled;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ssize_t size = sprintf(buf, "%lu %lu\n", total_mSchedutil_time, total_governor_time);

    total_mSchedutil_time = 0;
    total_governor_time = 0;
    
    return size;
}
 
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    total_mSchedutil_time = 0;
    total_governor_time = 0;
    
    return count;
}
 
struct kobject *kobj_pkshin2;
struct kobj_attribute overhead_attr = __ATTR(overhead, 0664, sysfs_show, sysfs_store);

static int __init pkshin_init(void) {
    kobj_pkshin2 = kobject_create_and_add("pkshin2", kernel_kobj);
    
    if(sysfs_create_file(kobj_pkshin2, &overhead_attr.attr)){
        printk(KERN_INFO"Cannot create sysfs file\n");
        sysfs_remove_file(kobj_pkshin2, &overhead_attr.attr);
        kobject_put(kobj_pkshin2); 
        return -1;
    }

    pkshin_overhead_enabled = 1;

    return 0;
}
static void __exit pkshin_exit(void) {
    sysfs_remove_file(kobj_pkshin2, &overhead_attr.attr);
    kobject_put(kobj_pkshin2);

    pkshin_overhead_enabled = 0;

    return;
}

module_init(pkshin_init);
module_exit(pkshin_exit);
