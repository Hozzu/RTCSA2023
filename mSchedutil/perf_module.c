#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Philkyue Shin <pkshin@redwood.snu.ac.kr>");
MODULE_DESCRIPTION("A module that enables performance aware schedutil governor");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

extern int pkshin_perf_enabled;

static int __init pkshin_init(void) {
    pkshin_perf_enabled = 1;

    return 0;
}
static void __exit pkshin_exit(void) {
    pkshin_perf_enabled = 0;
    
    return;
}

module_init(pkshin_init);
module_exit(pkshin_exit);
