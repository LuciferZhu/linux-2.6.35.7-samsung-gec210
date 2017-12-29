/*
 * spi board info base on s5pv210.
 *
 * Copyright (C) 2017 Lucifer Zhu <LuciferZhu@yeah.net>
 * date: 2017-12-4 23:41:33
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#define DEBUG
#include <linux/kernel.h>
#include <linux/device.h>
#undef DEBUG			/* avoid redefined DEBUG */
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/io.h>
/* driver specific headers */
#include <linux/spi/spi.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <plat-gec210/spi_s5p.h>


struct s5p_spi_csinfo gec_spi0_csinfo[] = {
	[0] = {
		.fb_delay = 0x00,
		.line = S5PV210_GPB(1),
		.set_level = gpio_set_value,
	},
};

struct s5p_spi_csinfo gec_spi1_csinfo[] = {
	[0] = {
		.fb_delay = 0x00,
		.line = S5PV210_GPB(5),
		.set_level = gpio_set_value,
	},
};


struct spi_board_info s5p_spi_devs[] __initdata = {
	[0] = {
		.modalias	= "spi0_dev",
		//const void	*platform_data;
		.controller_data= &gec_spi0_csinfo[0],
		.irq		= IRQ_SPI0,
		.max_speed_hz	= 10000000,
		.bus_num	= 0,
		//.chip_select	= ,
		.mode		= SPI_MODE_0,
	},
	[1] = {
		.modalias	= "mpu6500_spi",
		//const void	*platform_data;
		.controller_data= &gec_spi1_csinfo[0],
		.irq		= IRQ_SPI1,
		.max_speed_hz	= 1000000,
		.bus_num	= 1,
		//.chip_select	= ,
		.mode		= SPI_MODE_3,	/* CPOL=1, CPHA=1 */
	},
};


static int __init s5p_spi_board_init(void)
{
	pr_debug("%s()\n", __func__);
	return spi_register_board_info(s5p_spi_devs, ARRAY_SIZE(s5p_spi_devs));
}

static void __exit s5p_spi_board_exit(void)
{
	pr_debug("%s()\n", __func__);
}

module_init(s5p_spi_board_init);
module_exit(s5p_spi_board_exit);

MODULE_AUTHOR("Lucifer.Zhu, <LuciferZhu@yeah.net>");
MODULE_DESCRIPTION("spi board info base on s5pv210");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");

