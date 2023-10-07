#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>


static int array_leds[10]={};
bool state_leds[10]={};
static unsigned int led_count = 0;


static int my_led_probe(struct platform_device *pdev) {
    //int ret_val, irq;
    struct device *dev = &pdev->dev;

    pr_info("my_led_probe enter\n");
    dev_info(dev, "my_led_probe() function is called.\n");
    of_property_read_u32(pdev->dev.of_node,"id_led",&array_leds[led_count]);
    dev_info(dev," led %d \n",array_leds[led_count]);
    state_leds[led_count]=0;
    ++led_count;
    return 0;
}

static int  my_led_remove(struct platform_device *pdev) {
    return 0;
}

static const struct of_device_id my_led_of_ids[] = {
        { .compatible = "training,ledplatform"},
        {},
};

static ssize_t my_dev_write(struct file *flip, const char *buf, size_t len, loff_t *off)
{
    char kbuffer[250];
    int ret;
    u8 i;
    char cmd[10];
    pr_info("Write operation...\n");
  if(len < 250){
    ret = copy_from_user(kbuffer, buf, len);
    kbuffer[len]='\0';
    if(ret > 0){
      pr_err("error copy from user, %d char not copied \n",ret);
      return len - ret;
    }
    for(i=0; i<led_count; ++i){
      sprintf(cmd,"on%d",i);
      if(strcmp(kbuffer,cmd) == 0){
        pr_info("led[%d] allumé\n",i);
        gpio_set_value(array_leds[i], 1);
        state_leds[i]=1;
        break;
      }
      sprintf(cmd,"off%d",i);
      if(strcmp(kbuffer,cmd) == 0){
        pr_info("led[%d] etteint\n",i);
        gpio_set_value(array_leds[i], 0);
        state_leds[i]=0;
        break;
      }
      sprintf(cmd,"push%d",i);
      if(strcmp(kbuffer,cmd) == 0){
        state_leds[i] = !state_leds[i];
        pr_info("button pushed for led[%d], new state: %d\n",i,state_leds[i]);
        gpio_set_value(array_leds[i], state_leds[i]);
        break;
      }

    }
/*
    if(strcmp(kbuffer,"on1") == 0){
      pr_info("green led on\n");
      gpio_set_value(array_leds[0], 1);
    }
    else if (strcmp(kbuffer, "on2") == 0){
      gpio_set_value(array_leds[1], 1);
      pr_info("red led on\n");
    }
    else if(strcmp(kbuffer,"off1") == 0){
      gpio_set_value(array_leds[0], 0);
      pr_info("green led off\n");
    }
    else if (strcmp(kbuffer, "off2") == 0){
      gpio_set_value(array_leds[1], 0);
      pr_info("red led off\n");
    }
*/
  }
    return len;
}

/* Declare a file_operations structure */
static const struct file_operations my_dev_fops = {
  .owner = THIS_MODULE,
  .write = my_dev_write,
};


MODULE_DEVICE_TABLE(of, my_led_of_ids);

static struct miscdevice my_miscdevice = {
        .fops = &my_dev_fops,
        .minor = MISC_DYNAMIC_MINOR,
        .name = "project_leds", /*"mydev", nom dans /dev/ */
        .mode = 0666,
};


static struct platform_driver my_led_platform_driver = {
        .probe = my_led_probe,
        .remove = my_led_remove,
        .driver = {
                .name = "my_led",
                .of_match_table = my_led_of_ids,
                .owner = THIS_MODULE,
        }
};


static int __init my_init(void) {
    int ret_val;

    ret_val = platform_driver_register(&my_led_platform_driver);
    if (ret_val !=0)
    {
        pr_err("platform led value returned %d\n", ret_val);
        return ret_val;
    }


    pr_info("Misc registering...\n");
    ret_val = misc_register(&my_miscdevice);
    if (ret_val != 0) {
            pr_err("could not register the misc device mydev\n");
            return ret_val;
    }


    return 0;
}

static void __exit my_exit(void) {
    pr_info("Deregister misc device...\n");
    misc_deregister(&my_miscdevice);
    platform_driver_unregister(&my_led_platform_driver);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("C.A. ABID");
MODULE_DESCRIPTION("Leds handler module");
