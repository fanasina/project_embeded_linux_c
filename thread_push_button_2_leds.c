#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/of.h>

#include <linux/delay.h>
#include <linux/kthread.h>

size_t nb_devices = 0;
int irqs[10];
size_t irq_count[10] = {0};
bool led_state[10] = {0};

int button_pushed[10] = {0};

unsigned red_gpio=6, green_gpio=5;
unsigned led_gpio[10];
int red=0, green=1;

static DECLARE_WAIT_QUEUE_HEAD(my_wq);

struct task_struct *t1;
struct task_struct *t2;

int clignote_led(void *d){
  unsigned color = *(unsigned*)d;
  while(!kthread_should_stop()){
    while(button_pushed[color] == 1  && !kthread_should_stop()){
        gpio_set_value(led_gpio[color], led_state[color]);
        led_state[color] = !led_state[color];
        msleep(2000);
    }
    wait_event_interruptible(my_wq, button_pushed[color] == 1);
  }
  return 0;
}

static char *KEYS_NAME = "PB_KEY";
static ssize_t my_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
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
  .read = my_read,
};

/* Interrupt handler */
static irqreturn_t hello_keys_isr(int irq, void *data)
{ 
    /*static bool value_red = 0;
    static bool value_green = 0;*/

    struct device *dev = data;
    
    for(int i=0; i<nb_devices; ++i){
      if(irq == irqs[i]){
        ++irq_count[i];
        led_state[i] = !led_state[i];
        button_pushed[i] = !button_pushed[i];
        if(button_pushed[i]){
          wake_up_interruptible(&my_wq);
        }
      } 
    }
#if 0
    if(irq == irqs[0]){
      gpio_set_value(red_gpio, led_state[0]);
      /*gpio_set_value(red_gpio, value_red);
      value_red = !value_red;*/
    }
    if(irq == irqs[1]){      
      gpio_set_value(green_gpio, led_state[1]);
      /*gpio_set_value(green_gpio, value_green);
      value_green = !value_green;*/
    }
#endif
    dev_info(dev, "interrupt received. key: %s, irq: %d\n", KEYS_NAME,irq);
    return IRQ_HANDLED;
}

static struct miscdevice my_miscdevice = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "mydev",
        .fops = &fops,
        .mode = 0666,
};

static int my_probe(struct platform_device *pdev) {
    int ret_val,irq;

    struct device *dev = &pdev->dev;
    pr_info("my_probe enter\n");
    dev_info(dev, "my_probe() function is called.\n");

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
    dev_info(dev, "my_probe() function is exited.\n");
    return 0;
}

static int  my_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Remove called\n");
    return 0;
}

static const struct of_device_id my_of_ids[] = {
        { .compatible = "training,intbutton"},
        {},
};

MODULE_DEVICE_TABLE(of, my_of_ids);
static struct platform_driver my_platform_driver = {
        .probe = my_probe,
        .remove = my_remove,
        .driver = {
                .name = "int_button",
                .of_match_table = my_of_ids,
                .owner = THIS_MODULE,
        }
};

static int __init my_init(void) {
    int ret_val;
    pr_info("Push button driver enter\n");
    ret_val = platform_driver_register(&my_platform_driver);
    if (ret_val !=0)
    {
        pr_err("platform value returned %d\n", ret_val);
        return ret_val;
    }
    ret_val = misc_register(&my_miscdevice);
    if(ret_val != 0){
      pr_err("misc register value ret %d\n",ret_val);
      return ret_val;
    }

    if(gpio_is_valid(red_gpio) == 0){
      gpio_free(red_gpio);
      pr_err("gpio %d is now free \n", red_gpio);
    }
    if(gpio_is_valid(green_gpio) == 0){
      gpio_free(green_gpio);
      pr_err("gpio %d is now free \n", green_gpio);
    }

    led_gpio[0]=red_gpio;
    led_gpio[1]=green_gpio;

    gpio_request(red_gpio, "red");
    gpio_direction_output(red_gpio, 1);
    
    gpio_request(green_gpio, "green");
    gpio_direction_output(green_gpio, 1);
    
    led_state[0]=1;
    led_state[1]=1;
 
    
    t1 = kthread_run(clignote_led,&red,"clignote red");
    t2 = kthread_run(clignote_led,&green,"clignote green");

    return 0;
}

static void __exit my_exit(void) {
    pr_info("Push button driver exit\n");
    kthread_stop(t1);
    kthread_stop(t2);
    misc_deregister(&my_miscdevice);
    platform_driver_unregister(&my_platform_driver);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("C.A. ABID");
MODULE_DESCRIPTION("Using interrupts with a push button");

