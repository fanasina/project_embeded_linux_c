#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>


unsigned red_gpio=22, green_gpio=27;

static int my_dev_open(struct inode *inode, struct file *file)
{
  pr_info("my_dev_open() is called.\n");
  return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
  pr_info("my_dev_close() is called.\n");
  return 0;
}

bool is_read = false;

static ssize_t my_dev_read(struct file *filp, char *buf, size_t count, loff_t *pos) 
{
  return 0;
}

static ssize_t my_dev_write(struct file *flip, const char *buf, size_t len, loff_t *off) 
{    
    char kbuffer[250];
    int ret;
    pr_info("Write operation...\n");
  if(len < 250){ 
    ret = copy_from_user(kbuffer, buf, len);
    kbuffer[len]='\0';
    if(ret > 0){
      pr_err("error copy from user, %d char not copied \n",ret);
      return len - ret;
    }
    if(strcmp(kbuffer,"on1") == 0){
      pr_info("green led on\n");
      gpio_set_value(green_gpio, 1);
    }
    else if (strcmp(kbuffer, "on2") == 0){
      gpio_set_value(red_gpio, 1);
      pr_info("red led on\n");
    }
    else if(strcmp(kbuffer,"off1") == 0){
      gpio_set_value(green_gpio, 0);
      pr_info("green led off\n");
    }
    else if (strcmp(kbuffer, "off2") == 0){
      gpio_set_value(red_gpio, 0);
      pr_info("red led off\n");
    }

  }
    return len;
}


/* Declare a file_operations structure */
static const struct file_operations my_dev_fops = {
  .owner = THIS_MODULE,
  .open = my_dev_open,
  .release = my_dev_close,
  .write = my_dev_write,
  .read = my_dev_read,
};


static struct miscdevice led_misc_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "my_led",
  .fops = &my_dev_fops,
};



static int __init init_dev(void)
{
  int ret;
 
  led_misc_dev.mode = 0666; //S_IRUGO | S_IRWXUGO | S_IALLUGO 
  ret = misc_register(&led_misc_dev);

  if(ret != 0){
    pr_err("could not register the misc \n");
    return ret;
  }
  
  if(gpio_is_valid(red_gpio) == 0){
    gpio_free(red_gpio);
    pr_err("gpio %d is now free \n", red_gpio);
  }
  if(gpio_is_valid(green_gpio) == 0){
    gpio_free(green_gpio);
    pr_err("gpio %d is now free \n", green_gpio);
  }

  gpio_request(red_gpio, "red");
  gpio_direction_output(red_gpio, 1);
  
  
  gpio_request(green_gpio, "green");
  gpio_direction_output(green_gpio, 1);





  pr_info(" init led drive executed \n");

  return 0;
}

static void __exit exit_dev(void)
{

  gpio_set_value(red_gpio, 0);
  gpio_set_value(green_gpio, 0);

  gpio_free(red_gpio);
  gpio_free(green_gpio);

  misc_deregister(&led_misc_dev);

  pr_info(" exit led drive \n");
}



module_init(init_dev);
module_exit(exit_dev);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fanasina");
MODULE_DESCRIPTION(" module cdev!");
