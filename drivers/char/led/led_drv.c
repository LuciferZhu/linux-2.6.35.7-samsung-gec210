
//cdev driver
//author:Lucifer Zhu, 2016-12-10 17:46:53
//e-mail:18027858019@163.com

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/gpio.h>

#define GEC210_LED_MAJOR 200
#define GEC210_LED_MINOR 0
#define GEC210_LED_MINOR_CNT 1
//ioctl cmd's generate
#define GEC210_LED_ON _IOW('L',0x01,unsigned int)
#define GEC210_LED_OFF _IOW('L',0x02,unsigned int)

//1)定义一个字符设备
static struct cdev led_drv;

static dev_t gec210_led_dev_num;
//static struct resource *  gec210_led_res;
//static unsigned int *gpj2con_va; //0xe0200280对应的虚拟地址指针
//static unsigned int *gpj2dat_va; //0xe0200284对应的虚拟地址指针
static struct class *gec210_led_class;

struct led_gpio{
	unsigned int gpio_num;
	char gpio_name[12];	
};

static struct led_gpio gec210_leds[4] = {
	{
		.gpio_num = S5PV210_GPJ2(0),
		.gpio_name = "GPJ2_0-LED0",
	},
	{
		.gpio_num = S5PV210_GPJ2(1),
		.gpio_name = "GPJ2_1-LED1",
	},
	{
		.gpio_num = S5PV210_GPJ2(2),
		.gpio_name = "GPJ2_2-LED2",
	},
	{
		.gpio_num = S5PV210_GPJ2(3),
		.gpio_name = "GPJ2_3-LED3",
	},
};

static int gec210_led_open( struct inode *inode, struct file *filp )
{
	printk( "openning the driver of gec210_led\n" );
	return 0;
}

static ssize_t gec210_led_write( struct file *filp, const char __user *buf, size_t len, loff_t *ppos )
{
	char kbuf[2]; //kbuf[0]--> 1/0,on/off;  kbuf[1]--> LED 0/1/2/3
	int ret;
	
	if( len>2 )
		return -EFAULT;
	ret = copy_from_user( kbuf, buf, len);
	if( 0 != ret )
		return -EFAULT;
	if( kbuf[1]<0 || kbuf[1]>3 )
		return -EINVAL;

	if( 1 == kbuf[0] )
		//*gpj2dat_va &= ~(1<<kbuf[1]); //相应端口清0
		gpio_set_value( gec210_leds[kbuf[1]].gpio_num,0x0 );
	else if( 0 == kbuf[0] )
		//*gpj2dat_va |= (1<<kbuf[1]); //相应端口置位
		gpio_set_value( gec210_leds[kbuf[1]].gpio_num,0x1 );
	else
		return -EINVAL;
	
	printk( "<gec210_led_write>: kbuf[1]=%d,kbuf[0]=%d\n",kbuf[1],kbuf[0] );
	return len;
}

static int gec210_led_release( struct inode *inode, struct file *filp )
{
	//*gpj2dat_va |= 0xf; //turn off all led
	int i;
	for( i=0;i<4;i++ ){
		gpio_set_value( gec210_leds[i].gpio_num,0x1 );
	}
	printk( "closing the driver of gec210_led\n" );
	return 0;
}

static long gec210_led_ioctl( struct file *file,unsigned int cmd,unsigned long arg )
{
	if( arg>=sizeof(gec210_leds)/sizeof(struct led_gpio) ){
		return -EINVAL;
	}
	switch(cmd){
	case GEC210_LED_ON:
		gpio_set_value( gec210_leds[arg].gpio_num,0x0 );
		return 0;
	case GEC210_LED_OFF:
		gpio_set_value( gec210_leds[arg].gpio_num,0x1 );	
		return 0;
	default:
		return -ENOIOCTLCMD;
	}
}

static const struct file_operations gec210_led_fops = {
	.owner = THIS_MODULE,
	.open = gec210_led_open,
	.write = gec210_led_write,
	.release = gec210_led_release,
	.unlocked_ioctl = gec210_led_ioctl,
};

static int __init gec210_led_init(void) //driver's init and setup fcnt
{
	int ret,i;
	struct device * led_device;

	//2)申请/注册设备号
	if(GEC210_LED_MAJOR == 0){
		ret = alloc_chrdev_region(&gec210_led_dev_num, GEC210_LED_MINOR, GEC210_LED_MINOR_CNT, "gec210_led");
	}
	else{
		gec210_led_dev_num = MKDEV(GEC210_LED_MAJOR,GEC210_LED_MINOR);
		ret = register_chrdev_region( gec210_led_dev_num, GEC210_LED_MINOR_CNT, "gec210_led");
	}	
	if( ret<0 ){
		printk("gec210_led_dev_num is error \n");
		return ret;
	}
	
	//4)初始化cdev
	cdev_init( &led_drv, &gec210_led_fops);
	//5)将cdev加入kernel
	ret = cdev_add( &led_drv, gec210_led_dev_num, GEC210_LED_MINOR_CNT);
	if(ret < 0){
		printk("cdev add error\n");
		goto failed_cdev_add;
	}
	
	/*//6) 申请物理内存区，作为一个资源
	gec210_led_res = request_mem_region( 0xE0200280,8,"GPJ2_LED" );//GPJ2CON = 0xE020_0280, GPJ2DAT = 0xE020_0284
	if( NULL == gec210_led_res ){
		printk("requst mem region error\n");
		ret = -EBUSY;
		goto failed_request_mem_region;
	}

	//7) IO内存的动态映射
	gpj2con_va = ioremap( 0xE0200280,8 );
	if( NULL == gpj2con_va ){
		printk("ioremap error\n");
		ret = -EFAULT;
		goto failed_ioremap;
	}
	gpj2dat_va = gpj2con_va + 1;
	printk("gpj2con_va=%p, gpj2dat_va=%p\n", gpj2con_va,gpj2dat_va);*/
	
	//8) 创建class
	gec210_led_class = class_create(THIS_MODULE, "led_class");
	if (IS_ERR(gec210_led_class)) {
		printk("class_create error\n");
		ret = PTR_ERR(gec210_led_class);
		goto failed_class_create;
	}

	//9) 创建device
	led_device = device_create( gec210_led_class, NULL,
				gec210_led_dev_num, NULL, "led_drv");// /dev/led_drv
	if (IS_ERR(led_device)) {
		printk("device_create error\n");
		ret = PTR_ERR(led_device);
		goto failed_device_create;
	}

	//申请标准接口GPIO
	for( i=0;i<4;i++ ){
		ret = gpio_request( gec210_leds[i].gpio_num,gec210_leds[i].gpio_name );
		if (ret < 0) {
			printk("gpio_request error=%s\n",gec210_leds[i].gpio_name);
			goto fail_gpio_request;
		}
		gpio_direction_output(gec210_leds[i].gpio_num,0x1 ); //输出模式，并置位关灯
	}
	//设置GPIO为输出高电平，关灯
	/**gpj2con_va &= ~0xffff;
	*gpj2con_va |= 0x1111;
	*gpj2dat_va |= 0xff; //turn off led*/
	printk( "GEC210's led_drv boot succ!\n" );
	
	return 0;

fail_gpio_request:
	while(i--)
		gpio_free( gec210_leds[i].gpio_num );
	device_destroy(gec210_led_class,gec210_led_dev_num);
failed_device_create:
	class_destroy(gec210_led_class);
failed_class_create:
/*	iounmap( gpj2con_va );//取消内存映射,参数为虚拟地址
failed_ioremap:
	release_mem_region(0xe0200280,8);//释放物理内存区
failed_request_mem_region:*/
	cdev_del( &led_drv );
failed_cdev_add:	
	unregister_chrdev_region(gec210_led_dev_num,  GEC210_LED_MINOR_CNT);
	return ret;
}

static void __exit gec210_led_exit(void) //driver's exit fcnt
{		
	int i;
	//释放标准接口GPIO
	for( i=0;i<4;i++ ){
		gpio_free( gec210_leds[i].gpio_num );
	}
	device_destroy(gec210_led_class,gec210_led_dev_num);
	class_destroy(gec210_led_class);
	/*iounmap( gpj2con_va );//取消内存映射,参数为虚拟地址
	release_mem_region(0xe0200280,8);//释放物理内存区*/
	cdev_del( &led_drv );
	unregister_chrdev_region( gec210_led_dev_num, GEC210_LED_MINOR_CNT);
	
	printk( "good bye GEC210's led_drv\n" );
}


module_init( gec210_led_init ); //driver's entrance
module_exit( gec210_led_exit ); //driver's exit

//kernel module's description
MODULE_AUTHOR("Lucifer Zhu, <18027858019@163.com>");
MODULE_DESCRIPTION("led_drv base on GEC210");
MODULE_LICENSE("GPL"); //license with GPL protocol
MODULE_VERSION("V1.0");

