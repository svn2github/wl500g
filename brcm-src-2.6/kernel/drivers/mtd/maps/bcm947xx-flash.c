/*
 * Flash mapping for BCM947XX boards
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcm947xx-flash.c,v 1.5 2008/03/25 01:27:49 Exp $
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <siutils.h>
#include <trxhdr.h>

/* Global SB handle */
extern void *bcm947xx_sih;
extern spinlock_t bcm947xx_sih_lock;

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

#ifdef CONFIG_MTD_PARTITIONS
extern struct mtd_partition * init_mtd_partitions(struct mtd_info *mtd, size_t size);
#endif

#define WINDOW_ADDR 0x1fc00000
#define WINDOW_SIZE 0x400000
#define BUSWIDTH 2

/* e.g., flash=2M or flash=4M */
static int flash = 0;
module_param(flash, int, 0);
static int __init
bcm947xx_setup(char *str)
{
	flash = memparse(str, &str);
	return 1;
}
__setup("flash=", bcm947xx_setup);

static struct mtd_info *bcm947xx_mtd;


#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_bcm947xx_map init_module
#define cleanup_bcm947xx_map cleanup_module
#endif

struct map_info bcm947xx_map = {
	.name = "Physically mapped flash",
	.size = WINDOW_SIZE,
	.bankwidth = BUSWIDTH
};

static int __init
init_bcm947xx_map(void)
{
	ulong flags;
 	uint coreidx;
	chipcregs_t *cc;
	uint32 fltype;
	uint window_addr = 0, window_size = 0;
	size_t size;
	int ret = 0;
#ifdef CONFIG_MTD_PARTITIONS
	struct mtd_partition *parts;
#endif

	spin_lock_irqsave(&sih_lock, flags);
	coreidx = si_coreidx(sih);

	/* Check strapping option if chipcommon exists */
	if ((cc = si_setcore(sih, CC_CORE_ID, 0))) {
		fltype = readl(&cc->capabilities) & CC_CAP_FLASH_MASK;
		if (fltype == PFLASH) {
			bcm947xx_map.map_priv_2 = 1;
			window_addr = 0x1c000000;
			bcm947xx_map.size = window_size = 32 * 1024 * 1024;
			if ((readl(&cc->flash_config) & CC_CFG_DS) == 0)
				bcm947xx_map.bankwidth = 1;
		}
	} else {
		fltype = PFLASH;
		bcm947xx_map.map_priv_2 = 0;
		window_addr = WINDOW_ADDR;
		bcm947xx_map.size = window_size = WINDOW_SIZE;
	}

	si_setcoreidx(sih, coreidx);
	spin_unlock_irqrestore(&sih_lock, flags);

	if (fltype != PFLASH) {
		printk(KERN_ERR "pflash: found no supported devices\n");
		ret = -ENODEV;
		goto fail;
	}

	bcm947xx_map.virt = ioremap(window_addr, window_size);
	if (bcm947xx_map.virt == NULL) {
		printk(KERN_ERR "pflash: ioremap failed\n");
		ret = -EIO;
		goto fail;
	}

	if ((bcm947xx_mtd = do_map_probe("cfi_probe", &bcm947xx_map)) == NULL) {
		printk(KERN_ERR "pflash: cfi_probe failed\n");
		ret = -ENXIO;
		goto fail;
	}

	bcm947xx_mtd->owner = THIS_MODULE;

	/* Allow size override for testing */
	size = flash ? : bcm947xx_mtd->size;

	printk(KERN_NOTICE "Flash device: 0x%x at 0x%x\n", size, window_addr);

#ifdef CONFIG_MTD_PARTITIONS
	parts = init_mtd_partitions(bcm947xx_mtd, size);
	ret = add_mtd_partitions(bcm947xx_mtd, parts, 4);
	if (ret) {
		printk(KERN_ERR "pflash: add_mtd_partitions failed\n");
		goto fail;
	}
#endif

	return 0;

 fail:
	if (bcm947xx_mtd)
		map_destroy(bcm947xx_mtd);
	if (bcm947xx_map.map_priv_1)
		iounmap((void *) bcm947xx_map.map_priv_1);
	bcm947xx_map.map_priv_1 = 0;
	return ret;
}

static void __exit
cleanup_bcm947xx_map(void)
{
#ifdef CONFIG_MTD_PARTITIONS
	del_mtd_partitions(bcm947xx_mtd);
#endif
	map_destroy(bcm947xx_mtd);
	iounmap((void *) bcm947xx_map.map_priv_1);
	bcm947xx_map.map_priv_1 = 0;
}

module_init(init_bcm947xx_map);
module_exit(cleanup_bcm947xx_map);
