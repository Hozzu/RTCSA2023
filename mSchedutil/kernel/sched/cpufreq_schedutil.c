/*
 * CPUFreq governor based on scheduler-provided CPU utilization data.
 *
 * Copyright (C) 2016, Intel Corporation
 * Author: Rafael J. Wysocki <rafael.j.wysocki@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpufreq.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <trace/events/power.h>

#include "sched.h"
#include "tune.h"

#include <linux/ktime.h>

unsigned long boosted_cpu_util(int cpu);

/* Stub out fast switch routines present on mainline to reduce the backport
 * overhead. */
#define cpufreq_driver_fast_switch(x, y) 0
#define cpufreq_enable_fast_switch(x)
#define cpufreq_disable_fast_switch(x)
#define LATENCY_MULTIPLIER			(1000)
#define SUGOV_KTHREAD_PRIORITY	50

#define SUGOV_KTHREAD_PRIORITY	50

/* This is the maximum allowed value of capacity margin knob present in sysfs */
#define CAPACITY_MARGIN_MAX_VALUE		(1024)
#define CAPACITY_MARGIN_FLOOR_FACTOR		(10)

struct sugov_tunables {
	struct gov_attr_set attr_set;
	unsigned long iowait_boost_max;
	unsigned int up_rate_limit_us;
	unsigned int down_rate_limit_us;

	/*
	 * This knob can be used to control the aggressiveness of schedutil
	 * frequency scaling. Increasing the value will make scaling more
	 * aggressive.
	 */
	unsigned int capacity_margin;
};

struct sugov_policy {
	struct cpufreq_policy *policy;

	struct sugov_tunables *tunables;
	struct list_head tunables_hook;

	raw_spinlock_t update_lock;  /* For shared policies */
	u64 last_freq_update_time;
	s64 min_rate_limit_ns;
	s64 up_rate_delay_ns;
	s64 down_rate_delay_ns;
	unsigned int next_freq;
	unsigned int cached_raw_freq;

	/* The next fields are only needed if fast switch cannot be used. */
	struct irq_work irq_work;
	struct kthread_work work;
	struct mutex work_lock;
	struct kthread_worker worker;
	struct task_struct *thread;
	bool work_in_progress;

	bool need_freq_update;
};

struct sugov_cpu {
	struct update_util_data update_util;
	struct sugov_policy *sg_policy;

	unsigned long iowait_boost;
	u64 last_update;

	/* The fields below are only needed when sharing a policy. */
	unsigned long util;
	unsigned long max;
	unsigned int flags;

	/* The field below is for single-CPU policies only. */
#ifdef CONFIG_NO_HZ_COMMON
	unsigned long saved_idle_calls;
#endif
};

static DEFINE_PER_CPU(struct sugov_cpu, sugov_cpu);

//pkshin start
static int work_core = 3;

// for utilization tracking
int pkshin_util_track = 0;
EXPORT_SYMBOL(pkshin_util_track);
int pkshin_core_util_track = 0;
EXPORT_SYMBOL(pkshin_core_util_track);
int pkshin_memory_util_track = 0;
EXPORT_SYMBOL(pkshin_memory_util_track);
int pkshin_memory_track = 0;
EXPORT_SYMBOL(pkshin_memory_track);
int pkshin_track_enabled = 0;
EXPORT_SYMBOL(pkshin_track_enabled);

//for per-entity memory tracking
extern int pkshin_pemt_enabled;

//for memory-aware schedutil governor
int pkshin_freq_enabled = 0;
EXPORT_SYMBOL(pkshin_freq_enabled);

//for cycles tracking
extern int pkshin_cycle_enabled;
extern u64 exec_time[];
extern u64 exec_clear_time[];

u64 total_idle_time_sum = 0;
u64 total_busy_time_sum = 0;
u64 total_idle_cycle_sum = 0;
u64 total_busy_cycle_sum = 0;
EXPORT_SYMBOL(total_idle_time_sum);
EXPORT_SYMBOL(total_busy_time_sum);
EXPORT_SYMBOL(total_idle_cycle_sum);
EXPORT_SYMBOL(total_busy_cycle_sum);

//for overhead tracking
int pkshin_overhead_enabled = 0;
u64 total_mSchedutil_time = 0;
u64 total_governor_time = 0;
EXPORT_SYMBOL(pkshin_overhead_enabled);
EXPORT_SYMBOL(total_mSchedutil_time);
EXPORT_SYMBOL(total_governor_time);

//for memory and perf-aware schedutil governor
int pkshin_perf_enabled = 0;
EXPORT_SYMBOL(pkshin_perf_enabled);

//pkshin end

/************************ Governor internals ***********************/

static bool sugov_should_update_freq(struct sugov_policy *sg_policy, u64 time)
{
	s64 delta_ns;

	if (sg_policy->work_in_progress)
		return false;

	if (unlikely(sg_policy->need_freq_update)) {
		sg_policy->need_freq_update = false;
		/*
		 * This happens when limits change, so forget the previous
		 * next_freq value and force an update.
		 */
		sg_policy->next_freq = UINT_MAX;
		return true;
	}

	delta_ns = time - sg_policy->last_freq_update_time;

	/* No need to recalculate next freq for min_rate_limit_us at least */
	return delta_ns >= sg_policy->min_rate_limit_ns;
}

static bool sugov_up_down_rate_limit(struct sugov_policy *sg_policy, u64 time,
				     unsigned int next_freq)
{
	s64 delta_ns;

	delta_ns = time - sg_policy->last_freq_update_time;

	if (next_freq > sg_policy->next_freq &&
	    delta_ns < sg_policy->up_rate_delay_ns)
			return true;

	if (next_freq < sg_policy->next_freq &&
	    delta_ns < sg_policy->down_rate_delay_ns)
			return true;

	return false;
}

static void sugov_update_commit(struct sugov_policy *sg_policy, u64 time,
				unsigned int next_freq)
{
	struct cpufreq_policy *policy = sg_policy->policy;

	if (sugov_up_down_rate_limit(sg_policy, time, next_freq))
		return;

	if (sg_policy->next_freq == next_freq)
		return;

	sg_policy->next_freq = next_freq;
	sg_policy->last_freq_update_time = time;

	//ktime_t start, end;
	//start = ktime_get();
	if (policy->fast_switch_enabled) {
		next_freq = cpufreq_driver_fast_switch(policy, next_freq);
		if (next_freq == CPUFREQ_ENTRY_INVALID)
			return;

		policy->cur = next_freq;
		trace_cpu_frequency(next_freq, smp_processor_id());
	} else {
		sg_policy->work_in_progress = true;
		irq_work_queue(&sg_policy->irq_work);
		sg_policy->last_freq_update_time = time;
	}
	//end = ktime_get();
	//trace_printk("freq trans time is %u\n", ktime_to_ns(ktime_sub(end, start)));	
}

/**
 * get_next_freq - Compute a new frequency for a given cpufreq policy.
 * @sg_policy: schedutil policy object to compute the new frequency for.
 * @util: Current CPU utilization.
 * @max: CPU capacity.
 *
 * If the utilization is frequency-invariant, choose the new frequency to be
 * proportional to it, that is
 *
 * next_freq = C * max_freq * util / max
 *
 * Otherwise, approximate the would-be frequency-invariant utilization by
 * util_raw * (curr_freq / max_freq) which leads to
 *
 * next_freq = C * curr_freq * util_raw / max
 *
 * By default, take C = 1.25 for the frequency tipping point at (util / max) =
 * 0.8. The value of C can be changed indirectly via sysfs knob
 * 'capacity_margin'
 *
 * The lowest driver-supported frequency which is equal or greater than the raw
 * next_freq (as calculated above) is returned, subject to policy min/max and
 * cpufreq driver limitations.
 */
static unsigned int get_next_freq(struct sugov_policy *sg_policy, int cpu,
				  unsigned long util, unsigned long max)
{
	struct cpufreq_policy *policy = sg_policy->policy;
	unsigned int capacity_margin = sg_policy->tunables->capacity_margin;
	unsigned int freq = arch_scale_freq_invariant() ?
				policy->cpuinfo.max_freq : policy->cur;
	
	//trace_printk("capacity_margin is %lu\n", capacity_margin);//1024
	//trace_printk("arch_scale_freq_invariant is %d\n", arch_scale_freq_invariant());//1

	/*
	 * Cast freq to 64-bit so that we do not have to check for overflow in
	 * the calculations later on
	 */
	uint64_t tfreq = (uint64_t) freq;

//pkshin start
	if(pkshin_perf_enabled){
		struct rq *rq = cpu_rq(cpu);
		unsigned long scale_freq = arch_scale_freq_capacity(NULL, cpu);

		capacity_margin = capacity_margin * 2 * rq->cfs.avg.core_util_avg / (rq->cfs.avg.core_util_avg + (rq->cfs.avg.memory_util_avg * scale_freq >> 10));
	}
//pkshin end

	tfreq = (((tfreq * capacity_margin) >> CAPACITY_MARGIN_FLOOR_FACTOR) +
			tfreq) * util / max;

	/*
	 * tfreq will always be less than UINT_MAX as long as freq is less than
	 * UINT_MAX / 2
	 */
	freq = (unsigned int) tfreq;

	if (freq == sg_policy->cached_raw_freq && sg_policy->next_freq != UINT_MAX)
		return sg_policy->next_freq;

	sg_policy->cached_raw_freq = freq;

	return cpufreq_driver_resolve_freq(policy, freq);
}

static inline bool use_pelt(void)
{
#ifdef CONFIG_SCHED_WALT
	return (!sysctl_sched_use_walt_cpu_util || walt_disabled);
#else
	return true;
#endif
}

static void sugov_get_util(int cpu, unsigned long *util, unsigned long *max, u64 time)
{
//pkshin start
	//int cpu = smp_processor_id();
	struct rq *rq = cpu_rq(cpu);
//pkshin end
	unsigned long max_cap, rt;
	s64 delta;

	max_cap = arch_scale_cpu_capacity(NULL, cpu);

	sched_avg_update(rq);
	delta = time - rq->age_stamp;
	if (unlikely(delta < 0))
		delta = 0;
	rt = div64_u64(rq->rt_avg, sched_avg_period() + delta);
	rt = (rt * max_cap) >> SCHED_CAPACITY_SHIFT;

	*util = boosted_cpu_util(cpu);
	if (likely(use_pelt())) {
#ifdef CONFIG_TASK_WEIGHT
		*util = max(rq->cfs.avg.scaling_avg,
				rq->cfs.avg.scaling_fast_avg);
		//trace_printk("CONFIG_TASK WEIGHT is set\n");//set
#else
		*util = max(rq->cfs.avg.util_avg, rq->cfs.avg.util_fast_avg);
#endif
//pkshin start
		/**util = rq->cfs.avg.util_fast_avg;*/
		
		*util = rq->cfs.avg.util_avg;
		*util = min((*util + rt), max_cap);

		if(pkshin_pemt_enabled){
			// u64 start_time, end_time;
			// start_time = ktime_get_ns();

			unsigned long pkshin_util = max_cap;
			
			struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);
			struct sugov_policy *sg_policy = sg_cpu->sg_policy;

			unsigned long capacity_margin = sg_policy->tunables->capacity_margin;
			if(pkshin_perf_enabled){
				unsigned long scale_freq = arch_scale_freq_capacity(NULL, cpu);
				capacity_margin = capacity_margin * 2 * rq->cfs.avg.core_util_avg / (rq->cfs.avg.core_util_avg + (rq->cfs.avg.memory_util_avg * scale_freq >> 10));
			}

			/*unsigned long temp = (rq->cfs.avg.memory_util_fast_avg * capacity_margin >> CAPACITY_MARGIN_FLOOR_FACTOR) + rq->cfs.avg.memory_util_fast_avg;
			if(max_cap > temp){
				temp = max_cap - temp;
				if(temp > rq->cfs.avg.core_util_fast_avg)
					pkshin_util = max_cap * rq->cfs.avg.core_util_fast_avg / temp;
				else
					pkshin_util = max_cap;
			}*/

			unsigned long temp = (rq->cfs.avg.memory_util_avg * capacity_margin >> CAPACITY_MARGIN_FLOOR_FACTOR) + rq->cfs.avg.memory_util_avg;
			if(max_cap > temp){
				temp = max_cap - temp;
				if(temp > rq->cfs.avg.core_util_avg){
					pkshin_util = max_cap * rq->cfs.avg.core_util_avg / temp;
					pkshin_util = min((pkshin_util + rt), max_cap);
				}
			}

			// end_time = ktime_get_ns();
			// if(pkshin_overhead_enabled){
			// 	total_mSchedutil_time += end_time - start_time;
			// }

			//trace_printk("pkshin iter_num %llu elapsed_msched_time in schedutil code %llu\n",iter_num, elapsed_msched_time);
			//iter_num++;
	
			if(pkshin_freq_enabled){
					*util = pkshin_util;
			}
		}
//pkshin end
	}

	*max = max_cap;
}

static void sugov_set_iowait_boost(struct sugov_cpu *sg_cpu, u64 time,
				   unsigned int flags)
{
	unsigned long boost_max = sg_cpu->sg_policy->tunables->iowait_boost_max;

	if (flags & SCHED_CPUFREQ_IOWAIT) {
		sg_cpu->iowait_boost = boost_max;
	} else if (sg_cpu->iowait_boost) {
		s64 delta_ns = time - sg_cpu->last_update;

		/* Clear iowait_boost if the CPU apprears to have been idle. */
		if (delta_ns > TICK_NSEC)
			sg_cpu->iowait_boost = 0;
	}
}

static void sugov_iowait_boost(struct sugov_cpu *sg_cpu, unsigned long *util,
			       unsigned long *max)
{
	unsigned long boost_util = sg_cpu->iowait_boost;
	unsigned long boost_max = sg_cpu->sg_policy->tunables->iowait_boost_max;

	if (!boost_util)
		return;

	if (*util * boost_max < *max * boost_util) {
		*util = boost_util;
		*max = boost_max;
	}
	sg_cpu->iowait_boost >>= 1;
}

#ifdef CONFIG_NO_HZ_COMMON
static bool sugov_cpu_is_busy(struct sugov_cpu *sg_cpu)
{
	unsigned long idle_calls = tick_nohz_get_idle_calls();
	bool ret = idle_calls == sg_cpu->saved_idle_calls;

	sg_cpu->saved_idle_calls = idle_calls;
	return ret;
}
#else
static inline bool sugov_cpu_is_busy(struct sugov_cpu *sg_cpu) { return false; }
#endif /* CONFIG_NO_HZ_COMMON */

static void sugov_update_single(struct update_util_data *hook, u64 time,
				unsigned int flags)
{
	struct sugov_cpu *sg_cpu = container_of(hook, struct sugov_cpu, update_util);
	struct sugov_policy *sg_policy = sg_cpu->sg_policy;
	struct cpufreq_policy *policy = sg_policy->policy;
	unsigned long util, max;
	unsigned int next_f;
	bool busy;

	sugov_set_iowait_boost(sg_cpu, time, flags);
	sg_cpu->last_update = time;

	if (!sugov_should_update_freq(sg_policy, time))
		return;

	busy = sugov_cpu_is_busy(sg_cpu);

	if (flags & SCHED_CPUFREQ_DL) {
		//trace_printk("DL task is running. Freq is set to max.\n");
		next_f = policy->cpuinfo.max_freq;
	} else {
		sugov_get_util(smp_processor_id(), &util, &max, time);
		sugov_iowait_boost(sg_cpu, &util, &max);
		next_f = get_next_freq(sg_policy, smp_processor_id(), util, max);
		/*
		 * Do not reduce the frequency if the CPU has not been idle
		 * recently, as the reduction is likely to be premature then.
		 */
		if (busy && next_f < sg_policy->next_freq)
			next_f = sg_policy->next_freq;
	}
	sugov_update_commit(sg_policy, time, next_f);
}

static unsigned int sugov_next_freq_shared(struct sugov_cpu *sg_cpu, u64 time)
{
	struct sugov_policy *sg_policy = sg_cpu->sg_policy;
	struct cpufreq_policy *policy = sg_policy->policy;
	u64 last_freq_update_time = sg_policy->last_freq_update_time;
	unsigned long util = 0, max = 1;
	unsigned int j;
//pkshin start
	unsigned int new_shared_core = 0;
//pkshin end	

//	for_each_cpu(j, policy->cpus) {
		j = work_core;
		struct sugov_cpu *j_sg_cpu = &per_cpu(sugov_cpu, j);
		unsigned long j_util, j_max;
		s64 delta_ns;

		/*
		 * If the CPU utilization was last updated before the previous
		 * frequency update and the time elapsed between the last update
		 * of the CPU utilization and the last frequency update is long
		 * enough, don't take the CPU into account as it probably is
		 * idle now (and clear iowait_boost for it).
		 */
		/*delta_ns = last_freq_update_time - j_sg_cpu->last_update;
		if (delta_ns > TICK_NSEC) {
			j_sg_cpu->iowait_boost = 0;
			continue;
		}*/
		if (j_sg_cpu->flags & SCHED_CPUFREQ_DL){
			//trace_printk("DL task is running. Freq is set to max.\n");
			return policy->cpuinfo.max_freq;
		}
//pkshin start
		sugov_get_util(j, &j_util, &j_max, time);
		/*j_util = j_sg_cpu->util;
		j_max = j_sg_cpu->max;*/
//pkshin end

		if (j_util * max > j_max * util) {
			util = j_util;
			max = j_max;
//pkshin start			
			new_shared_core = j;
//pkshin end			
		}
		sugov_iowait_boost(j_sg_cpu, &util, &max);
	//}
//pkshin start
	unsigned int next_freq = get_next_freq(sg_policy, new_shared_core, util, max);
	struct rq *rq;
	rq = cpu_rq(new_shared_core);
	trace_printk("next_freq is %u, util_avg is %u, pkshin_util is %u, core_util is %u, mem_util is %u\n", next_freq, rq->cfs.avg.util_avg, rq->cfs.avg.core_util_avg + rq->cfs.avg.memory_util_avg * next_freq / policy->cpuinfo.max_freq, rq->cfs.avg.core_util_avg, rq->cfs.avg.memory_util_avg);

	if(pkshin_track_enabled){
		int i = work_core;
		pkshin_util_track = util;
		pkshin_core_util_track = cpu_rq(i)->cfs.avg.core_util_avg;
		pkshin_memory_util_track = cpu_rq(i)->cfs.avg.memory_util_avg;
		pkshin_memory_track = cpu_rq(i)->cfs.avg.memory_cycle_ratio;
	}

	if(pkshin_cycle_enabled){
		int i = work_core;
		u64 now = rq_clock_task(cpu_rq(i));
		u64 interval = now - exec_clear_time[i];
		interval >>= 10;//us

		if(interval){
			unsigned long scale_freq = arch_scale_freq_capacity(NULL, i);

			if(interval > exec_time[i]){
				total_idle_time_sum += (interval - exec_time[i]);
				total_idle_cycle_sum += (interval - exec_time[i]) * scale_freq;
			}

			total_busy_time_sum += exec_time[i];
			total_busy_cycle_sum += exec_time[i] * scale_freq;

			exec_clear_time[i] = now;
			exec_time[i] = 0;
		}	
	}

	return next_freq;
//pkshin end
}

static void sugov_update_shared(struct update_util_data *hook, u64 time,
				unsigned int flags)
{
	struct sugov_cpu *sg_cpu = container_of(hook, struct sugov_cpu, update_util);
	struct sugov_policy *sg_policy = sg_cpu->sg_policy;
	unsigned long util, max;
	unsigned int next_f;

//pkshin start
	// u64 start_time2, end_time2;
	// start_time2 = ktime_get_ns();
	//sugov_get_util(&util, &max, time);

	raw_spin_lock(&sg_policy->update_lock);

	//sg_cpu->util = util;
	//sg_cpu->max = max;
	sg_cpu->flags = flags;
//pkshin end

	sugov_set_iowait_boost(sg_cpu, time, flags);
	sg_cpu->last_update = time;

	if (sugov_should_update_freq(sg_policy, time)) {
		if (flags & SCHED_CPUFREQ_DL){
			//trace_printk("DL task is running. Freq is set to max.\n");
			next_f = sg_policy->policy->cpuinfo.max_freq;
		}
		else
			next_f = sugov_next_freq_shared(sg_cpu, time);

		sugov_update_commit(sg_policy, time, next_f);
	}

	raw_spin_unlock(&sg_policy->update_lock);

	// end_time2 = ktime_get_ns();
	// if(pkshin_overhead_enabled){
	// 	total_governor_time += end_time2 - start_time2;
	// }
}

static void sugov_work(struct kthread_work *work)
{
	struct sugov_policy *sg_policy = container_of(work, struct sugov_policy, work);

	mutex_lock(&sg_policy->work_lock);
	__cpufreq_driver_target(sg_policy->policy, sg_policy->next_freq,
				CPUFREQ_RELATION_L);
	mutex_unlock(&sg_policy->work_lock);

	sg_policy->work_in_progress = false;
}

static void sugov_irq_work(struct irq_work *irq_work)
{
	struct sugov_policy *sg_policy;

	sg_policy = container_of(irq_work, struct sugov_policy, irq_work);

	/*
	 * For RT and deadline tasks, the schedutil governor shoots the
	 * frequency to maximum. Special care must be taken to ensure that this
	 * kthread doesn't result in the same behavior.
	 *
	 * This is (mostly) guaranteed by the work_in_progress flag. The flag is
	 * updated only at the end of the sugov_work() function and before that
	 * the schedutil governor rejects all other frequency scaling requests.
	 *
	 * There is a very rare case though, where the RT thread yields right
	 * after the work_in_progress flag is cleared. The effects of that are
	 * neglected for now.
	 */
	kthread_queue_work(&sg_policy->worker, &sg_policy->work);
}

/************************** sysfs interface ************************/

static struct sugov_tunables *global_tunables;
static DEFINE_MUTEX(global_tunables_lock);

static inline struct sugov_tunables *to_sugov_tunables(struct gov_attr_set *attr_set)
{
	return container_of(attr_set, struct sugov_tunables, attr_set);
}

static DEFINE_MUTEX(min_rate_lock);

static void update_min_rate_limit_us(struct sugov_policy *sg_policy)
{
	mutex_lock(&min_rate_lock);
	sg_policy->min_rate_limit_ns = min(sg_policy->up_rate_delay_ns,
					   sg_policy->down_rate_delay_ns);
	mutex_unlock(&min_rate_lock);
}

static ssize_t up_rate_limit_us_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	return sprintf(buf, "%u\n", tunables->up_rate_limit_us);
}

static ssize_t down_rate_limit_us_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	return sprintf(buf, "%u\n", tunables->down_rate_limit_us);
}

static ssize_t up_rate_limit_us_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int rate_limit_us;

	if (kstrtouint(buf, 10, &rate_limit_us))
		return -EINVAL;

	tunables->up_rate_limit_us = rate_limit_us;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		sg_policy->up_rate_delay_ns = rate_limit_us * NSEC_PER_USEC;
		update_min_rate_limit_us(sg_policy);
	}

	return count;
}

static ssize_t iowait_boost_max_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	return sprintf(buf, "%lu\n", tunables->iowait_boost_max);
}

static ssize_t iowait_boost_max_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned long iowait_boost_max;

	if (kstrtoul(buf, 10, &iowait_boost_max))
		return -EINVAL;

	tunables->iowait_boost_max = iowait_boost_max;

	return count;
}

static ssize_t capacity_margin_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	return sprintf(buf, "%u\n", tunables->capacity_margin);
}

static ssize_t capacity_margin_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int capacity_margin;

	if (kstrtouint(buf, 10, &capacity_margin))
		return -EINVAL;

	if (capacity_margin > CAPACITY_MARGIN_MAX_VALUE) {
		return -EINVAL;
	}

	tunables->capacity_margin = capacity_margin;

	return count;
}

static ssize_t down_rate_limit_us_store(struct gov_attr_set *attr_set,
					const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int rate_limit_us;

	if (kstrtouint(buf, 10, &rate_limit_us))
		return -EINVAL;

	tunables->down_rate_limit_us = rate_limit_us;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		sg_policy->down_rate_delay_ns = rate_limit_us * NSEC_PER_USEC;
		update_min_rate_limit_us(sg_policy);
	}

	return count;
}

static struct governor_attr iowait_boost_max = __ATTR_RW(iowait_boost_max);
static struct governor_attr up_rate_limit_us = __ATTR_RW(up_rate_limit_us);
static struct governor_attr down_rate_limit_us = __ATTR_RW(down_rate_limit_us);
static struct governor_attr capacity_margin = __ATTR_RW(capacity_margin);

static struct attribute *sugov_attributes[] = {
	&up_rate_limit_us.attr,
	&down_rate_limit_us.attr,
	&iowait_boost_max.attr,
	&capacity_margin.attr,
	NULL
};

static struct kobj_type sugov_tunables_ktype = {
	.default_attrs = sugov_attributes,
	.sysfs_ops = &governor_sysfs_ops,
};

/********************** cpufreq governor interface *********************/

static struct cpufreq_governor schedutil_gov;

static struct sugov_policy *sugov_policy_alloc(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy;

	sg_policy = kzalloc(sizeof(*sg_policy), GFP_KERNEL);
	if (!sg_policy)
		return NULL;

	sg_policy->policy = policy;
	raw_spin_lock_init(&sg_policy->update_lock);
	return sg_policy;
}

static void sugov_policy_free(struct sugov_policy *sg_policy)
{
	kfree(sg_policy);
}

static int sugov_kthread_create(struct sugov_policy *sg_policy)
{
	struct task_struct *thread;
	struct sched_param param = { .sched_priority = MAX_USER_RT_PRIO / 2 };
	struct cpufreq_policy *policy = sg_policy->policy;
	int ret;

	/* kthread only required for slow path */
	if (policy->fast_switch_enabled)
		return 0;

	kthread_init_work(&sg_policy->work, sugov_work);
	kthread_init_worker(&sg_policy->worker);
	thread = kthread_create(kthread_worker_fn, &sg_policy->worker,
				"sugov:%d",
				cpumask_first(policy->related_cpus));
	if (IS_ERR(thread)) {
		pr_err("failed to create sugov thread: %ld\n", PTR_ERR(thread));
		return PTR_ERR(thread);
	}

	ret = sched_setscheduler_nocheck(thread, SCHED_FIFO, &param);
	if (ret) {
		kthread_stop(thread);
		pr_warn("%s: failed to set SCHED_FIFO\n", __func__);
		return ret;
	}

	sg_policy->thread = thread;
	kthread_bind_mask(thread, policy->related_cpus);
	init_irq_work(&sg_policy->irq_work, sugov_irq_work);
	mutex_init(&sg_policy->work_lock);

	wake_up_process(thread);

	return 0;
}

static void sugov_kthread_stop(struct sugov_policy *sg_policy)
{
	/* kthread only required for slow path */
	if (sg_policy->policy->fast_switch_enabled)
		return;

	kthread_flush_worker(&sg_policy->worker);
	kthread_stop(sg_policy->thread);
	mutex_destroy(&sg_policy->work_lock);
}

static struct sugov_tunables *sugov_tunables_alloc(struct sugov_policy *sg_policy)
{
	struct sugov_tunables *tunables;

	tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
	if (tunables) {
		gov_attr_set_init(&tunables->attr_set, &sg_policy->tunables_hook);
		if (!have_governor_per_policy())
			global_tunables = tunables;
	}
	return tunables;
}

static void sugov_tunables_free(struct sugov_tunables *tunables)
{
	if (!have_governor_per_policy())
		global_tunables = NULL;

	kfree(tunables);
}

static int sugov_init(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy;
	struct sugov_tunables *tunables;
	unsigned int lat;
	int ret = 0;

	/* State should be equivalent to EXIT */
	if (policy->governor_data)
		return -EBUSY;

	cpufreq_enable_fast_switch(policy);

	sg_policy = sugov_policy_alloc(policy);
	if (!sg_policy) {
		ret = -ENOMEM;
		goto disable_fast_switch;
	}

	ret = sugov_kthread_create(sg_policy);
	if (ret)
		goto free_sg_policy;

	mutex_lock(&global_tunables_lock);

	if (global_tunables) {
		if (WARN_ON(have_governor_per_policy())) {
			ret = -EINVAL;
			goto stop_kthread;
		}
		policy->governor_data = sg_policy;
		sg_policy->tunables = global_tunables;

		gov_attr_set_get(&global_tunables->attr_set, &sg_policy->tunables_hook);
		goto out;
	}

	tunables = sugov_tunables_alloc(sg_policy);
	if (!tunables) {
		ret = -ENOMEM;
		goto stop_kthread;
	}

	tunables->up_rate_limit_us = LATENCY_MULTIPLIER;
	tunables->down_rate_limit_us = LATENCY_MULTIPLIER;
	lat = policy->cpuinfo.transition_latency / NSEC_PER_USEC;
	if (lat) {
		tunables->up_rate_limit_us *= lat;
		tunables->down_rate_limit_us *= lat;
	}

	tunables->up_rate_limit_us = 500;
	tunables->down_rate_limit_us = 2000;
	tunables->capacity_margin = 256;

	policy->governor_data = sg_policy;
	sg_policy->tunables = tunables;

	ret = kobject_init_and_add(&tunables->attr_set.kobj, &sugov_tunables_ktype,
				   get_governor_parent_kobj(policy), "%s",
				   schedutil_gov.name);
	if (ret)
		goto fail;

out:
	mutex_unlock(&global_tunables_lock);
	return 0;

fail:
	policy->governor_data = NULL;
	sugov_tunables_free(tunables);

 stop_kthread:
	sugov_kthread_stop(sg_policy);

 free_sg_policy:
	mutex_unlock(&global_tunables_lock);

	sugov_policy_free(sg_policy);

disable_fast_switch:
	cpufreq_disable_fast_switch(policy);

	pr_err("initialization failed (error %d)\n", ret);
	return ret;
}

static void sugov_exit(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy = policy->governor_data;
	struct sugov_tunables *tunables = sg_policy->tunables;
	unsigned int count;

	mutex_lock(&global_tunables_lock);

	count = gov_attr_set_put(&tunables->attr_set, &sg_policy->tunables_hook);
	policy->governor_data = NULL;
	if (!count)
		sugov_tunables_free(tunables);

	mutex_unlock(&global_tunables_lock);

	sugov_kthread_stop(sg_policy);
	sugov_policy_free(sg_policy);
	cpufreq_disable_fast_switch(policy);
}

static int sugov_start(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy = policy->governor_data;
	unsigned int cpu;

	sg_policy->up_rate_delay_ns =
		sg_policy->tunables->up_rate_limit_us * NSEC_PER_USEC;
	sg_policy->down_rate_delay_ns =
		sg_policy->tunables->down_rate_limit_us * NSEC_PER_USEC;
	update_min_rate_limit_us(sg_policy);
	sg_policy->last_freq_update_time = 0;
	sg_policy->next_freq = UINT_MAX;
	sg_policy->work_in_progress = false;
	sg_policy->need_freq_update = false;
	sg_policy->tunables->iowait_boost_max = policy->cpuinfo.max_freq;
	sg_policy->cached_raw_freq = 0;

	for_each_cpu(cpu, policy->cpus) {
		struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);

		memset(sg_cpu, 0, sizeof(*sg_cpu));
		sg_cpu->sg_policy = sg_policy;
		sg_cpu->flags = SCHED_CPUFREQ_DL;
		cpufreq_add_update_util_hook(cpu, &sg_cpu->update_util,
					     policy_is_shared(policy) ?
							sugov_update_shared :
							sugov_update_single);
	}
	return 0;
}

static void sugov_stop(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy = policy->governor_data;
	unsigned int cpu;

	for_each_cpu(cpu, policy->cpus)
		cpufreq_remove_update_util_hook(cpu);

	synchronize_sched();

	if (!policy->fast_switch_enabled) {
		irq_work_sync(&sg_policy->irq_work);
		kthread_cancel_work_sync(&sg_policy->work);
	}
}

static void sugov_limits(struct cpufreq_policy *policy)
{
	struct sugov_policy *sg_policy = policy->governor_data;

	if (!policy->fast_switch_enabled) {
		mutex_lock(&sg_policy->work_lock);
		cpufreq_policy_apply_limits(policy);
		mutex_unlock(&sg_policy->work_lock);
	}

	sg_policy->need_freq_update = true;
}

static struct cpufreq_governor schedutil_gov = {
	.name = "schedutil",
	.owner = THIS_MODULE,
	.init = sugov_init,
	.exit = sugov_exit,
	.start = sugov_start,
	.stop = sugov_stop,
	.limits = sugov_limits,
};

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_SCHEDUTIL
struct cpufreq_governor *cpufreq_default_governor(void)
{
	return &schedutil_gov;
}
#endif

static int __init sugov_register(void)
{
	return cpufreq_register_governor(&schedutil_gov);
}
fs_initcall(sugov_register);
