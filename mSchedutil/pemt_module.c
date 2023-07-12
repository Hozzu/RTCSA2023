#include <linux/init.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/smp.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Philkyue Shin <pkshin@redwood.snu.ac.kr>");
MODULE_DESCRIPTION("A module that enables pemt(per-entity memory cycle ratio tracking)");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

extern int pkshin_pemt_enabled;

DECLARE_PER_CPU(u32, prevcc);
DECLARE_PER_CPU(u32, prevfs);
DECLARE_PER_CPU(u32, prevbs);
DECLARE_PER_CPU(u32, previn);
DECLARE_PER_CPU(u32, prevmb);

#define ARMV8_PMEVTYPER_EVTCOUNT_MASK  0x3ff
#define ARMV8_PMUSERENR_EN_EL0  (1 << 0) /* EL0 access enable */
#define ARMV8_PMUSERENR_CR      (1 << 2) /* Cycle counter read enable */
#define ARMV8_PMUSERENR_ER      (1 << 3) /* Event counter read enable */
#define ARMV8_PMCNTENSET_ENABLE (1 << 31) /* Enable cycle counter */
#define ARMV8_PMCNTENCLR_CLEAR 0xffffffff /* Disable all counters */
#define ARMV8_PMCR_E            (1 << 0) /* Enable all counters */
#define ARMV8_PMCR_P            (1 << 1) /* Reset all event counters */

static void disable_all_pmu(void * ignored) {
    u32 val = 0;
    /* Performance Monitors Count Enable Clear Register: clear all counters */
    //asm volatile("msr pmcntenclr_el0, %0" : : "r" ((u64)ARMV8_PMCNTENCLR_CLEAR));

    /* Performance Monitors Control Register: disable and reset all counters */
	//asm volatile("mrs %0, pmcr_el0" : "=r" (val));
	//asm volatile("msr pmcr_el0, %0" : : "r" ((val&(~ARMV8_PMCR_E))|ARMV8_PMCR_P));

    u32 * pvcc = &get_cpu_var(prevcc);
    *pvcc = 0;
    put_cpu_var(prevcc);

    u32 * pvfs = &get_cpu_var(prevfs);
    *pvfs = 0;
    put_cpu_var(prevfs);

    u32 * pvbs = &get_cpu_var(prevbs);
    *pvbs = 0;
    put_cpu_var(prevbs);

    u32 * pvin = &get_cpu_var(previn);
    *pvin = 0;
    put_cpu_var(previn);

    u32 * pvmb = &get_cpu_var(prevmb);
    *pvmb = 0;
    put_cpu_var(prevmb);
}

static void init_pmu(void) {
	u32 val = 0;
	/* Performance Monitors User Enable Register: enable user-mode access to all counters */
	asm volatile("msr pmuserenr_el0, %0" : : "r" ((u64)ARMV8_PMUSERENR_EN_EL0|ARMV8_PMUSERENR_ER|ARMV8_PMUSERENR_CR));

	/* Performance Monitors Count Enable Set register: enable cycle counter */
	asm volatile("msr pmcntenset_el0, %0" : : "r" ((u64)ARMV8_PMCNTENSET_ENABLE));

	/* Performance Monitors Control Register: enable and reset all counters */
	asm volatile("mrs %0, pmcr_el0" : "=r" (val));
	asm volatile("msr pmcr_el0, %0" : : "r" (val|ARMV8_PMCR_E|ARMV8_PMCR_P));
}

static void enable_pmu(u32 evtCount, u32 counterId) {
    u32 r = 0;
    evtCount &= ARMV8_PMEVTYPER_EVTCOUNT_MASK;
    asm volatile("isb");

    /* Performance Monitors Event Counter Selection Register: select pmevcntr<n> */
    asm volatile("msr pmselr_el0, %0" : : "r" (counterId));
    /* Performance Monitors Selected Event Type Register: select specific event counter */
    asm volatile("msr pmxevtyper_el0, %0" : : "r" (evtCount));

    /* Performance Monitors Count Enable Set register: enable specific event counter */
    asm volatile("mrs %0, pmcntenset_el0" : "=r" (r));
    asm volatile("msr pmcntenset_el0, %0" : : "r" (r|(1<<counterId)));
}

static void enable_pmu_want(void * ignored){
    init_pmu();
    enable_pmu(0x11, 1); // The number of CPU cycles
	enable_pmu(0x23, 2); // The number of frontend stall cycles
	enable_pmu(0x24, 3); // The number of backend stall cycles
    enable_pmu(0x08, 4); // The number of instruction retired
    enable_pmu(0x10, 5); // The number of mispredicted instructions

    u32 * pvcc = &get_cpu_var(prevcc);
    *pvcc = 0;
    put_cpu_var(prevcc);

    u32 * pvfs = &get_cpu_var(prevfs);
    *pvfs = 0;
    put_cpu_var(prevfs);

    u32 * pvbs = &get_cpu_var(prevbs);
    *pvbs = 0;
    put_cpu_var(prevbs);

    u32 * pvin = &get_cpu_var(previn);
    *pvin = 0;
    put_cpu_var(previn);

    u32 * pvmb = &get_cpu_var(prevmb);
    *pvmb = 0;
    put_cpu_var(prevmb);
}

static int __init pkshin_init(void) {
    on_each_cpu(enable_pmu_want, NULL, 1);
    pkshin_pemt_enabled = 1;

    return 0;
}
static void __exit pkshin_exit(void) {
    pkshin_pemt_enabled = 0;
    on_each_cpu(disable_all_pmu, NULL, 1);
    
    return;
}

module_init(pkshin_init);
module_exit(pkshin_exit);
