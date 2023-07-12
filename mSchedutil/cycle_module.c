#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>

MODULE_AUTHOR("Philkyue Shin <pkshin@redwood.snu.ac.kr>");
MODULE_DESCRIPTION("A module that enables cycle tracking");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

extern u64 total_idle_time_sum;
extern u64 total_busy_time_sum;
extern u64 total_idle_cycle_sum;
extern u64 total_busy_cycle_sum;

extern int pkshin_cycle_enabled;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    //ssize_t size = sprintf(buf, "p %lu e %lu d %lu n %lu\n", pred_cycle_sum, exec_cycle_sum, cycle_diff_sum, freq_change_num);
    ssize_t size = sprintf(buf, "%lu %lu %lu %lu\n", total_idle_time_sum, total_busy_time_sum, total_idle_cycle_sum, total_busy_cycle_sum);

    total_idle_time_sum = 0;
    total_busy_time_sum = 0;
    total_idle_cycle_sum = 0;
    total_busy_cycle_sum = 0;
    
    return size;
}
 
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    total_idle_time_sum = 0;
    total_busy_time_sum = 0;
    total_idle_cycle_sum = 0;
    total_busy_cycle_sum = 0;
    
    return count;
}
 
struct kobject *kobj_pkshin;
struct kobj_attribute cycle_attr = __ATTR(cycle, 0664, sysfs_show, sysfs_store);

static int __init pkshin_init(void) {
    kobj_pkshin = kobject_create_and_add("pkshin", kernel_kobj);
    
    if(sysfs_create_file(kobj_pkshin, &cycle_attr.attr)){
        printk(KERN_INFO"Cannot create sysfs file\n");
        sysfs_remove_file(kobj_pkshin, &cycle_attr.attr);
        kobject_put(kobj_pkshin); 
        return -1;
    }

    pkshin_cycle_enabled = 1;

    return 0;
}
static void __exit pkshin_exit(void) {
    sysfs_remove_file(kobj_pkshin, &cycle_attr.attr);
    kobject_put(kobj_pkshin);

    pkshin_cycle_enabled = 0;

    return;
}

module_init(pkshin_init);
module_exit(pkshin_exit);
