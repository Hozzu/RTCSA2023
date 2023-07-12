#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>

MODULE_AUTHOR("Philkyue Shin <pkshin@redwood.snu.ac.kr>");
MODULE_DESCRIPTION("A module that enables utilization and memory stall cycle ratio tracking");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

extern int pkshin_util_track;
extern int pkshin_core_util_track;
extern int pkshin_memory_util_track;
extern int pkshin_memory_track;
extern int pkshin_track_enabled;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ssize_t size = sprintf(buf, "%u %u %u %u\n", pkshin_util_track, pkshin_core_util_track, pkshin_memory_util_track, pkshin_memory_track);
    
    return size;
}
 
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    pkshin_util_track = 0;
    pkshin_core_util_track = 0;
    pkshin_memory_util_track = 0;
    pkshin_memory_track = 0;
    
    return count;
}
 
struct kobject *kobj_pkshin3;
struct kobj_attribute track_attr = __ATTR(track, 0664, sysfs_show, sysfs_store);

static int __init pkshin_init(void) {
    kobj_pkshin3 = kobject_create_and_add("pkshin3", kernel_kobj);
    
    if(sysfs_create_file(kobj_pkshin3, &track_attr.attr)){
        printk(KERN_INFO"Cannot create sysfs file\n");
        sysfs_remove_file(kobj_pkshin3, &track_attr.attr);
        kobject_put(kobj_pkshin3); 
        return -1;
    }

    pkshin_track_enabled = 1;

    return 0;
}
static void __exit pkshin_exit(void) {
    sysfs_remove_file(kobj_pkshin3, &track_attr.attr);
    kobject_put(kobj_pkshin3);

    pkshin_track_enabled = 0;

    return;
}

module_init(pkshin_init);
module_exit(pkshin_exit);
