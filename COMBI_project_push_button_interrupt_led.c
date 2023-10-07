#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
//#include <linux/gpio.h>
#include <linux/of.h>

#include <linux/unistd.h>

size_t nb_devices = 0;
int irqs[10];
size_t irq_count[10] = {0};
//bool led_state[10] = {0};

//unsigned red_gpio=6, green_gpio=5;


static char *KEYS_NAME = "PB_KEY";
static ssize_t my_int_b_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
{
  char kbuf[2000];
  size_t len;
  if(*ppos == 0){
    kbuf[0]='\0';
    for(int i=0; i<nb_devices; ++i){
      sprintf(kbuf,"%sid: %d :%ld\n",kbuf,irqs[i],irq_count[i]);
    }
    len = strlen(kbuf);
    if(copy_to_user(buff,kbuf,len) != 0) // return nb caractère non copier, si 0 c'est reussi
      return -EFAULT;
    ++(*ppos);
    return len;
  }
  return 0;
}

static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = my_int_b_read,
};

/* Interrupt handler */
static irqreturn_t hello_keys_isr(int irq, void *data)
{ 
    /*static bool value_red = 0;
    static bool value_green = 0;*/
    int i, fd;
    char cmd[20];
    struct device *dev = data;
    fd=open(dev_leds_name, 0_WRONLY);

    for(i=0;i<nb_devices;++i){
      if(irq == irqs[i]){
        ++irq_count[i];
        sprintf(cmd,"push%d",i);
        write(fd,cmd,strlen(cmd));
      }
    }
    dev_info(dev, "interrupt received. key: %s, irq: %d\n", KEYS_NAME,irq);
    return IRQ_HANDLED;
}

static struct miscdevice my_int_b_int_button_miscdevice = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "mydev",
        .fops = &fops,
        .mode = 0666,
};

static int my_int_b_probe(struct platform_device *pdev) {
    int ret_val,irq;

    struct device *dev = &pdev->dev;
    pr_info("my_int_b_probe enter\n");
    dev_info(dev, "my_int_b_probe() function is called.\n");

    /* Get the virtual Linux IRQ number */
    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        dev_err(dev, "irq is not available\n");
        return -EINVAL;
    }
    dev_info(dev, "IRQ_using_platform_get_irq: %d\n", irq);

    ret_val = devm_request_irq(dev, irq, hello_keys_isr, IRQF_SHARED, KEYS_NAME, dev);
    if (ret_val != 0)
    {
        dev_err(dev, "Error when resquest IRQ\n");
        return ret_val;
    }
    irqs[nb_devices++] = irq;
    dev_info(dev, "my_int_b_probe() function is exited.\n");
    return 0;
}

static int  my_int_b_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Remove called\n");
    return 0;
}

static const struct of_device_id my_int_b_of_ids[] = {
        { .compatible = "training,intbutton"},
        {},
};

MODULE_DEVICE_TABLE(of, my_int_b_of_ids);
static struct platform_driver my_int_b_int_button_platform_driver = {
        .probe = my_int_b_probe,
        .remove = my_int_b_remove,
        .driver = {
                .name = "int_button",
                .of_match_table = my_int_b_of_ids,
                .owner = THIS_MODULE,
        }
};

static int __init my_int_b_init(void) {
    int ret_val;
    pr_info("Push button driver enter\n");
    ret_val = platform_driver_register(&my_int_b_int_button_platform_driver);
    if (ret_val !=0)
    {
        pr_err("platform value returned %d\n", ret_val);
        return ret_val;
    }
    ret_val = misc_register(&my_int_b_int_button_miscdevice);
    if(ret_val != 0){
      pr_err("misc register value ret %d\n",ret_val);
      return ret_val;
    }
/*
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
    
    led_state[0]=1;
    led_state[1]=1;
*/
    return 0;
}

static void __exit my_int_b_exit(void) {
    pr_info("Push button driver exit\n");
    misc_deregister(&my_int_b_int_button_miscdevice);
    platform_driver_unregister(&my_int_b_int_button_platform_driver);
}

module_init(my_int_b_init);
module_exit(my_int_b_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("C.A. ABID");
MODULE_DESCRIPTION("Using interrupts with a push button");

