/*
 * HND MIPS boards setup routines
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: setup.c,v 1.6 2008/04/03 03:49:45 Exp $
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_core.h>
#if defined(CONFIG_BLK_DEV_IDE) || defined(CONFIG_BLK_DEV_IDE_MODULE)
#include <linux/blkdev.h>
#include <linux/ide.h>
#endif
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>
#include <asm/reboot.h>

#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/minix_fs.h>
#include <linux/ext2_fs.h>
#include <linux/romfs_fs.h>
#include <linux/cramfs_fs.h>
#include <linux/squashfs_fs.h>
#endif

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndcpu.h>
#include <mips33_core.h>
#include <mips74k_core.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <trxhdr.h>
#include "bcm947xx.h"

extern void bcm947xx_timer_setup(struct irqaction *irq);

#ifdef CONFIG_KGDB
extern void set_debug_traps(void);
extern void rs_kgdb_hook(struct uart_port *);
extern void breakpoint(void);
#endif

/* Global SB handle */
si_t *bcm947xx_sih = NULL;
spinlock_t bcm947xx_sih_lock = SPIN_LOCK_UNLOCKED;
EXPORT_SYMBOL(bcm947xx_sih);
EXPORT_SYMBOL(bcm947xx_sih_lock);

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

/* Kernel command line */
extern char arcs_cmdline[CL_SIZE];
static int lanports_enable = 0;
static int wombo_reset = GPIO_PIN_NOTDEFINED;

static void
bcm947xx_reboot_handler(void)
{
	if (lanports_enable) {
		uint lp = 1 << lanports_enable;

		si_gpioout(sih, lp, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, lp, lp, GPIO_DRV_PRIORITY);
		bcm_mdelay(1);
	}

	/* gpio 0 is also valid wombo_reset */
	if (wombo_reset != GPIO_PIN_NOTDEFINED) {
		int reset = 1 << wombo_reset;

		si_gpioout(sih, reset, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(10);
	}
}

void
bcm947xx_machine_restart(char *command)
{
	printk("Please stand by while rebooting the system...\n");

	/* Set the watchdog timer to reset immediately */
	local_irq_disable();
	bcm947xx_reboot_handler();
	hnd_cpu_reset(sih);
}

void
bcm947xx_machine_halt(void)
{
	printk("System halted\n");

	/* Disable interrupts and watchdog and spin forever */
	local_irq_disable();
	si_watchdog(sih, 0);
	bcm947xx_reboot_handler();
	while (1);
}

#ifdef CONFIG_SERIAL_CORE

static struct uart_port rs = {
	line: 0,
	flags: ASYNC_BOOT_AUTOCONF,
	iotype: SERIAL_IO_MEM,
};

static void __init
serial_add(void *regs, uint irq, uint baud_base, uint reg_shift)
{
	rs.membase = regs;
	rs.irq = irq + 2;
	rs.uartclk = baud_base;
	rs.regshift = reg_shift;

	early_serial_setup(&rs);

	rs.line++;
}

static void __init
serial_setup(si_t *sih)
{
	si_serial_init(sih, serial_add);

#ifdef CONFIG_KGDB
	/* Use the last port for kernel debugging */
	if (rs.membase)
		rs_kgdb_hook(&rs);
#endif
}

#endif /* CONFIG_SERIAL_CORE */

void __init
brcm_setup(void)
{
	char *value;

	/* Get global SB handle */
	sih = si_kattach(SI_OSH);

	/* Initialize clocks and interrupts */
	si_mips_init(sih, SBMIPS_VIRTIRQ_BASE);

	if (BCM330X(current_cpu_data.processor_id) &&
		(read_c0_diag() & BRCM_PFC_AVAIL)) {
		/* 
		 * Now that the sih is inited set the  proper PFC value 
		 */	
		printk("Setting the PFC to its default value\n");
		enable_pfc(PFC_AUTO);
	}


#ifdef CONFIG_SERIAL_CORE
	/* Initialize UARTs */
	serial_setup(sih);
#endif /* CONFIG_SERIAL_CORE */

	/* Override default command line arguments */
	value = nvram_get("kernel_args");
	if (value && strlen(value) && strncmp(value, "empty", 5))
		strncpy(arcs_cmdline, value, sizeof(arcs_cmdline));


	if ((lanports_enable = getgpiopin(NULL, "lanports_enable", GPIO_PIN_NOTDEFINED)) ==
		GPIO_PIN_NOTDEFINED)
		lanports_enable = 0;

	/* wombo reset */
	if ((wombo_reset = getgpiopin(NULL, "wombo_reset", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		int reset = 1 << wombo_reset;

		printk("wombo_reset set to gpio %d\n", wombo_reset);

		si_gpioout(sih, reset, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(10);

		si_gpioout(sih, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(20);
	}

	/* Generic setup */
	_machine_restart = bcm947xx_machine_restart;
	_machine_halt = bcm947xx_machine_halt;
	pm_power_off = bcm947xx_machine_halt;

}

const char *
get_system_type(void)
{
	static char s[32];

	if (bcm947xx_sih) {
		sprintf(s, "Broadcom BCM%X chip rev %d pkg %d", bcm947xx_sih->chip,
			bcm947xx_sih->chiprev, bcm947xx_sih->chippkg);
		return s;
	}
	else
		return "Broadcom BCM947XX";
}

void __init
bus_error_init(void)
{
}

void __init
plat_mem_setup(void)
{
	brcm_setup();
	return;
}

#ifdef CONFIG_MTD_PARTITIONS

static struct mtd_partition bcm947xx_parts[] =
{
	{
		.name = "boot",
		.size = 0,
		.offset = 0,
//		.mask_flags = MTD_WRITEABLE
	},
	{
		.name = "linux",
		.size = 0,
		.offset = 0
	},
	{
		.name = "rootfs",
		.size = 0,
		.offset = 0,
		.mask_flags = MTD_WRITEABLE
	},
	{
		.name = "nvram",
		.size = 0,
		.offset = 0
	}
};

struct mtd_partition *
init_mtd_partitions(struct mtd_info *mtd, size_t size)
{
	const int bufsz = BLOCK_SIZE;
	union {
		struct minix_super_block *minix;
		struct ext2_super_block *ext2;
		struct romfs_super_block *romfs;
		struct cramfs_super *cramfs;
		struct squashfs_super_block *squashfs;
		struct trx_header *trx;
		unsigned char *buf;
	} u;
	int off;
	size_t len;

	u.buf = kmalloc(bufsz, GFP_KERNEL);
	if (!u.buf)
		return NULL;

	/* Look at every 64 KB boundary */
	for (off = 0; off < size; off += (64 * 1024)) {
		memset(u.buf, 0xe5, bufsz);

		/*
		 * Read block 0 to test for superblocks
		 */
		if (mtd->read(mtd, off, bufsz, &len, u.buf) ||
		    len != bufsz)
			continue;

		/* Try looking at TRX header for rootfs offset */
		if (le32_to_cpu(u.trx->magic) == TRX_MAGIC) {
			bcm947xx_parts[1].offset = off;
//			if (le32_to_cpu(u.trx->offsets[1]) > off)
				if (le32_to_cpu(u.trx->offsets[2]) > off)
					off = le32_to_cpu(u.trx->offsets[2]);
				else if (le32_to_cpu(u.trx->offsets[1]) > off)
					off = le32_to_cpu(u.trx->offsets[1]);
			continue;
		}

#if defined(CONFIG_ROMFS_FS)
		/* romfs is at block zero too */
		if (u.romfs->word0 == ROMSB_WORD0 &&
		    u.romfs->word1 == ROMSB_WORD1) {
			printk(KERN_NOTICE
			       "%s: romfs filesystem found at block %d\n",
			       mtd->name, off >> BLOCK_SIZE_BITS);
			goto done;
		}
#endif

#if defined(CONFIG_SQUASHFS)
		/* squashfs is at block zero too */
		if (u.squashfs->s_magic == SQUASHFS_MAGIC
			|| u.squashfs->s_magic == SQUASHFS_MAGIC_LZMA) {
                        printk(KERN_NOTICE
                               "%s: squashfs filesystem found at block %d\n",
                               mtd->name, off >> BLOCK_SIZE_BITS);
                        goto done;
		}
#endif

#if defined(CONFIG_CRAMFS)
		/* so is cramfs */
		if (u.cramfs->magic == CRAMFS_MAGIC) {
			printk(KERN_NOTICE
			       "%s: cramfs filesystem found at block %d\n",
			       mtd->name, off >> BLOCK_SIZE_BITS);
			goto done;
		}
#endif

		/*
		 * Read block 1 to test for minix and ext2 superblock
		 */
		if (mtd->read(mtd, off + BLOCK_SIZE, bufsz, &len, u.buf) ||
		    len != bufsz)
			continue;

#if defined(CONFIG_MINIX_FS)
		/* Try minix */
		if (u.minix->s_magic == MINIX_SUPER_MAGIC ||
		    u.minix->s_magic == MINIX_SUPER_MAGIC2) {
			printk(KERN_NOTICE
			       "%s: minix filesystem found at block %d\n",
			       mtd->name, off >> BLOCK_SIZE_BITS);
			goto done;
		}
#endif

		/* Try ext2 */
		if (u.ext2->s_magic == cpu_to_le16(EXT2_SUPER_MAGIC)) {
			printk(KERN_NOTICE
			       "%s: ext2 filesystem found at block %d\n",
			       mtd->name, off >> BLOCK_SIZE_BITS);
			goto done;
		}

	}

	printk(KERN_NOTICE
	       "%s: Couldn't find valid ROM disk image\n",
	       mtd->name);

 done:
	/* Find and size nvram */
	bcm947xx_parts[3].offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
	bcm947xx_parts[3].size = size - bcm947xx_parts[3].offset;

	/* Find and size rootfs */
	if (off < size) {
		bcm947xx_parts[2].offset = off;
		bcm947xx_parts[2].size = bcm947xx_parts[3].offset - bcm947xx_parts[2].offset;
	}

	/* Size linux (kernel and rootfs) */
	bcm947xx_parts[1].size = bcm947xx_parts[3].offset - bcm947xx_parts[1].offset;

	/* Size pmon */
	bcm947xx_parts[0].size = bcm947xx_parts[1].offset - bcm947xx_parts[0].offset;

	kfree(u.buf);
	return bcm947xx_parts;
}

EXPORT_SYMBOL(init_mtd_partitions);

#endif
