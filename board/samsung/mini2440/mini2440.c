/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002, 2010
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

#define FCLK_SPEED 1

#if FCLK_SPEED==0		/* Fout = 203MHz, Fin = 12MHz for Audio */
#define M_MDIV	0xC3
#define M_PDIV	0x4
#define M_SDIV	0x1
#elif FCLK_SPEED==1		/* Fout = 202.8MHz */
#define M_MDIV	0xA1
#define M_PDIV	0x3
#define M_SDIV	0x1
#endif

#define USB_CLOCK 1

#if USB_CLOCK==0
#define U_M_MDIV	0xA1
#define U_M_PDIV	0x3
#define U_M_SDIV	0x1
#elif USB_CLOCK==1
#define U_M_MDIV	0x48
#define U_M_PDIV	0x3
#define U_M_SDIV	0x2
#endif

/* for mini2440 */
#define MVAL                        (0) 
#define MVAL_USED                   (0)          //0=each frame   1=rate by MVAL 
#define INVVDEN                     (1)          //0=normal       1=inverted 
#define BSWP                        (0)          //Byte swap control 
#define HWSWP                       (1)          //Half word swap control 

//TFT 240x320 
#define LCD_XSIZE_TFT_240320        (240)     
#define LCD_YSIZE_TFT_240320        (320) 

//TFT240320 
#define HOZVAL_TFT_240320           (LCD_XSIZE_TFT_240320-1) 
#define LINEVAL_TFT_240320          (LCD_YSIZE_TFT_240320-1) 

//Timing parameter for NEC3.5" 
#define VBPD_240320   (3)     
#define VFPD_240320   (10) 
#define VSPW_240320    (1) 

#define HBPD_240320    (5) 
#define HFPD_240320   (2) 
#define HSPW_240320_NEC    (36)  //Adjust the horizontal displacement of the screen :tekkamanninja@163.com 
#define HSPW_240320_TD       (23)  //64MB nand mini2440 is 36 ,128MB is 23 
#define CLKVAL_TFT_240320 (3) 
//FCLK=101.25MHz,HCLK=50.625MHz,VCLK=6.33MHz 
void board_video_init (GraphicDevice *pGD) 
{
    struct s3c24x0_lcd * const lcd     = s3c24x0_get_base_lcd(); 
    struct s3c2440_nand * const nand = s3c2440_get_base_nand(); 
    /* FIXME: select LCM type by env variable */ 
    /* Configuration for GTA01 LCM on QT2410 */ 
    lcd->lcdcon1 = 0x00000378; /* CLKVAL=4, BPPMODE=16bpp, TFT, ENVID=0 */ 
    lcd->lcdcon2 = (VBPD_240320<<24)|(LINEVAL_TFT_240320<<14)|(VFPD_240320<<6)|(VSPW_240320); 
    lcd->lcdcon3 = (HBPD_240320<<19)|(HOZVAL_TFT_240320<<8)|(HFPD_240320);
    
    if ( (nand->nfconf) & 0x08 )   { 
        lcd->lcdcon4 = (MVAL<<8)|(HSPW_240320_TD); 
    }
    else {
        lcd->lcdcon4 = (MVAL<<8)|(HSPW_240320_NEC); 
    }

    lcd->lcdcon5 = 0x00000f09; 
    lcd->lpcsel  = 0x00000000; 
    
}

static inline void pll_delay(unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_early_init_f(void)
{
	struct s3c24x0_clock_power * const clk_power =
					s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	writel(0xFFFFFF, &clk_power->locktime);

	/* configure MPLL */
	writel((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV,
	       &clk_power->mpllcon);

	/* some delay between MPLL and UPLL */
	pll_delay(4000);

	/* configure UPLL */
	writel((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV,
	       &clk_power->upllcon);

	/* some delay between MPLL and UPLL */
	pll_delay(8000);

	/* set up the I/O ports */
	/* set up gpa */
	writel(0x007FFFFF, &gpio->gpacon);

	/* set up gpb */
	writel(0x00044555, &gpio->gpbcon);
	writel(0x000007FF, &gpio->gpbup);

	/* set up gpc */
	writel(0xAAAAAAAA, &gpio->gpccon);
	writel(0x0000FFFF, &gpio->gpcup);
	
	/* set up gpd */
	writel(0xAAAAAAAA, &gpio->gpdcon);
	writel(0x0000FFFF, &gpio->gpdup);
	
	/* set up gpe */
	writel(0xAAAAAAAA, &gpio->gpecon);
	writel(0x0000FFFF, &gpio->gpeup);
	
	/* set up gpf */
	writel(0x000055AA, &gpio->gpfcon);
	writel(0x000000FF, &gpio->gpfup);
	
	/* set up gpg */
	writel(0xFF95FFBA, &gpio->gpgcon);
	writel(0x0000FFFF, &gpio->gpgup);
	
	/* set up gph */
	writel(0x002AFAAA, &gpio->gphcon);
	writel(0x000007FF, &gpio->gphup);

	return 0;
}

int board_init(void)
{
	/* arch number of MINI2440-Board */
	gd->bd->bi_arch_number = MACH_TYPE_MINI2440;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_CS8900
	rc = cs8900_initialize(0, CONFIG_CS8900_BASE);
#endif
#ifdef CONFIG_DRIVER_DM9000
        rc = dm9000_initialize(bis);
#endif

	return rc;
}
#endif

/*
 * Hardcoded flash setup:
 * Flash 0 is a non-CFI AMD AM29LV800BB flash.
 */
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	info->portwidth = FLASH_CFI_16BIT;
	info->chipwidth = FLASH_CFI_BY16;
	info->interface = FLASH_CFI_X16;
	return 1;
}
