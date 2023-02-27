/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <buffer.h>
#include <cpuid.h>
#include <debug.h>
#include <errno.h>
#include <gic.h>
#include <plat_import_sym.h>
#include <rmm_el3_ifc.h>
#include <sizes.h>
#include <stdint.h>
#include <string.h>
#include <xlat_contexts.h>
#include <xlat_tables.h>

/*
 * Memory map REGIONS used for the RMM runtime (static mappings)
 */
#define RMM_CODE_SIZE		(RMM_CODE_END - RMM_CODE_START)
#define RMM_RO_SIZE		(RMM_RO_END - RMM_RO_START)
#define RMM_RW_SIZE		(RMM_RW_END - RMM_RW_START)

#define RMM_CODE		MAP_REGION_FLAT(			\
					RMM_CODE_START,			\
					RMM_CODE_SIZE,			\
					MT_CODE | MT_REALM)

#define RMM_RO			MAP_REGION_FLAT(			\
					RMM_RO_START,			\
					RMM_RO_SIZE,			\
					MT_RO_DATA | MT_REALM)

#define RMM_RW			MAP_REGION_FLAT(			\
					RMM_RW_START,			\
					RMM_RW_SIZE,			\
					MT_RW_DATA | MT_REALM)

/*
 * Some of the fields for the RMM_SHARED region will be populated
 * at runtime.
 */
#define RMM_SHARED		MAP_REGION(				\
					0U,				\
					RMM_SHARED_BUFFER_START,	\
					0U,				\
					MT_RW_DATA | MT_REALM)

/* Number of common memory mapping regions */
#define COMMON_REGIONS		(4U)

/* Total number of memory mapping regions */
#define TOTAL_MMAP_REGIONS	(COMMON_REGIONS + PLAT_CMN_EXTRA_MMAP_REGIONS)

/* Memory mapping regions for the system runtime */
static struct xlat_mmap_region static_regions[TOTAL_MMAP_REGIONS];

/* Allocate the runtime translation tables */
static uint64_t static_s1tt[XLAT_TABLE_ENTRIES * PLAT_CMN_CTX_MAX_XLAT_TABLES]
					__aligned(XLAT_TABLES_ALIGNMENT)
					__section("xlat_static_tables");

/* Structures to hold the runtime translation context information  */
static struct xlat_ctx_tbls runtime_tbls;
static struct xlat_ctx_cfg runtime_xlat_ctx_cfg;
static struct xlat_ctx runtime_xlat_ctx;

/*
 * Platform common cold boot setup for RMM.
 *
 * This function should only be invoked once during cold boot
 * and is expected to setup architecture and platform components
 * common for all PEs executing RMM.
 * The xlat tables and GIC driver are initialized by this function.
 */
int plat_cmn_setup(unsigned long x0, unsigned long x1,
		   unsigned long x2, unsigned long x3,
		   struct xlat_mmap_region *plat_regions,
		   unsigned int nregions)
{
	int ret;
	unsigned int plat_offset, cmn_offset;

	(void)x0;
	(void)x1;
	(void)x2;
	(void)x3;

	/* Common regions sorted by ascending VA */
	struct xlat_mmap_region regions[COMMON_REGIONS] = {
		RMM_CODE,
		RMM_RO,
		RMM_RW,
		RMM_SHARED
	};

	if (nregions > PLAT_CMN_EXTRA_MMAP_REGIONS) {
		return -ERANGE;
	}

	if (nregions > 0U && plat_regions == NULL) {
		return -EINVAL;
	}

	/* Setup the parameters of the shared area */
	regions[3].base_pa = rmm_el3_ifc_get_shared_buf_pa();
	regions[3].size = rmm_el3_ifc_get_shared_buf_size();

	plat_offset = COMMON_REGIONS;
	cmn_offset = 0U;
	if (nregions > 0U) {
		/*
		 * Combine the common memory regions with the platform ones
		 * in an array where they are sorted as per VA.
		 */
		if (plat_regions[0].base_va < RMM_CODE_START) {
			plat_offset = 0U;
			cmn_offset = nregions;
		}
		(void)memcpy((void *)&static_regions[plat_offset],
			     (void *)&plat_regions[0U],
			     sizeof(struct xlat_mmap_region) * nregions);
	}

	(void)memcpy((void *)&static_regions[cmn_offset], (void *)&regions[0U],
		     sizeof(struct xlat_mmap_region) * COMMON_REGIONS);

	ret = xlat_ctx_cfg_init(&runtime_xlat_ctx_cfg, VA_LOW_REGION,
				&static_regions[0], nregions + COMMON_REGIONS,
				VIRT_ADDR_SPACE_SIZE);

	if (ret != 0) {
		ERROR("%s (%u): %s (%i)\n",
			__func__, __LINE__,
			"Failed to initialize the xlat ctx within the xlat library ",
			ret);
		return ret;
	}

	ret = xlat_ctx_init(&runtime_xlat_ctx, &runtime_xlat_ctx_cfg,
			    &runtime_tbls,
			    &static_s1tt[0],
			    PLAT_CMN_CTX_MAX_XLAT_TABLES);

	if (ret != 0) {
		ERROR("%s (%u): %s (%i)\n",
			__func__, __LINE__,
			"Failed to create the xlat ctx within the xlat library ",
			ret);
		return ret;
	}

	/* Read supported GIC virtualization features and init GIC variables */
	gic_get_virt_features();

	/* Perform coold boot initialization of the slot buffer mechanism */
	return slot_buf_coldboot_init();
}

/*
 * Local PE common platform setup for RMM.
 *
 * This function will only be invoked during
 * warm boot and is expected to setup architecture and platform
 * components local to a PE executing RMM.
 */
int plat_cmn_warmboot_setup(void)
{
	int ret;

	/* Setup the MMU cfg for the low region (runtime context) */
	ret = xlat_arch_setup_mmu_cfg(&runtime_xlat_ctx);
	if (ret != 0) {
		ERROR("%s (%u): Failed to setup xlat tables for CPU[%u]\n",
			__func__, __LINE__, my_cpuid());
		return ret;
	}

	/* Setup the MMU cfg for the slot buffer context (high region) */
	slot_buf_setup_xlat();

	VERBOSE("xlat tables configured for CPU[%u]\n", my_cpuid());
	return 0;
}
