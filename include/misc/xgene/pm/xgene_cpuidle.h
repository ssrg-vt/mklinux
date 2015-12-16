/*
 * xgene_cpuidle.h
 *
 *  Created on: May 17, 2013
 *      Author: ndhillon
 */

#ifndef XGENE_CPUIDLE_H_
#define XGENE_CPUIDLE_H_

#include <linux/cpuidle.h>

int xgene_enter_idle_simple(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index);
int xgene_enter_idle_coupled(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index);

int xgene_enter_standby(u32 disable_l2c_prefetch);
int xgene_clockgate_pmd(u32 pmd);
int xgene_clockenable_pmd(u32 pmd);
void xgene_cpuidle_update_states_desc(struct cpuidle_driver *drv);
void xgene_cpuidle_update_states_status(struct cpuidle_device *dev);

#endif /* XGENE_CPUIDLE_H_ */
