/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <host_rmi_wrappers.h>

/* Declaring SMC handler */
void handle_ns_smc(unsigned long function_id,
		   unsigned long arg0,
		   unsigned long arg1,
		   unsigned long arg2,
		   unsigned long arg3,
		   unsigned long arg4,
		   unsigned long arg5,
		   struct smc_result *ret);

void host_rmi_granule_delegate(void *granule_address, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_GRANULE_DELEGATE,
		(uintptr_t)granule_address,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_granule_undelegate(void *granule_address, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_GRANULE_UNDELEGATE,
		(uintptr_t)granule_address,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_realm_create(void *rd, void *params_ptr, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REALM_CREATE,
		(uintptr_t)rd,
		(uintptr_t)params_ptr,
		0, 0, 0, 0,
		ret);
}

void host_rmi_realm_destroy(void *rd, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REALM_DESTROY,
		(uintptr_t)rd,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_rtt_create(void *rtt, void *rd, void *ipa, unsigned int level, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_RTT_CREATE,
		(uintptr_t)rtt,
		(uintptr_t)rd,
		(uintptr_t)ipa,
		level,
		0, 0,
		ret);
}

void host_rmi_rtt_destroy(void *rtt, void *rd, void *ipa,
			  unsigned int level, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_RTT_DESTROY,
		(uintptr_t)rtt,
		(uintptr_t)rd,
		(uintptr_t)ipa,
		level,
		0, 0,
		ret);
}

void host_rmi_rec_aux_count(void *rd, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REC_AUX_COUNT,
		(uintptr_t)rd,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_rec_create(void *rec, void *rd, void *params_ptr, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REC_CREATE,
		(uintptr_t)rec,
		(uintptr_t)rd,
		(uintptr_t)params_ptr,
		0, 0, 0,
		ret);
}

void host_rmi_rec_destroy(void *rec, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REC_DESTROY,
		(uintptr_t)rec,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_realm_activate(void *rd, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REALM_ACTIVATE,
		(uintptr_t)rd,
		0, 0, 0, 0, 0,
		ret);
}

void host_rmi_rec_enter(void *rec, void *run_ptr, struct smc_result *ret)
{
	handle_ns_smc(SMC_RMM_REC_ENTER,
		(uintptr_t)rec,
		(uintptr_t)run_ptr,
		0, 0, 0, 0,
		ret);
}