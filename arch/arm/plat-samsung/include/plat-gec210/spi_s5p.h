#ifndef __SPI_S5P_H
#define __SPI_S5P_H __FILE__


#include <linux/platform_device.h>

struct s5p_spi_csinfo {
	u8 fb_delay;
	unsigned line;
	void (*set_level)(unsigned line_id, int lvl);
};


struct spi_s5p_platdata {
	int (*cfg_gpio)(struct platform_device *pdev);
};



#endif /*__SPI_S5P_H */
