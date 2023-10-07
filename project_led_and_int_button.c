#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>

#include <linux/delay.h>
#include <linux/kthread.h>

#define INIT_DELAY 1000

static int array_leds[10]={};
static int id_leds[10]={};
bool state_leds[10]={};
static int led_count = 0;

int nb_devices_int_b = 0;
int irqs[10];
size_t irq_count[10] = {0};

int button_pushed[10] = {0};

struct mutex lock_button[10];

size_t delay_led[10]={1000};
size_t delay_max = INIT_DELAY;
size_t delay_min = INIT_DELAY;

int altern = 0;

// =======================================================================================

size_t parse_char_to_int(char *input, int base){
   size_t len = strlen(input);
   size_t val = 0, p=1;
   long i;
   for(i=len-1; i>=0; --i)
      if(input[i] >='0' && input[i]<='9'){
        val += p * (input[i]-'0');
        p *= base;

      }
      else if(input[i] == ':')
        break;
   return val;
}

    


// ====================================== thread handler ===================================

struct task_struct *tsk[10];

static DECLARE_WAIT_QUEUE_HEAD(my_wq);


int clignote_led(void *d){
  unsigned rank_color = *(unsigned*)d;
  while(!kthread_should_stop()){
      while( /*rank_color < nb_devices_int_b &&*/ button_pushed[rank_color] == 1  && !kthread_should_stop()){
          //gpio_set_value(led_gpio[rank_color], led_state[rank_color]);
          //led_state[rank_color] = !led_state[rank_color];
          gpio_set_value(array_leds[rank_color], state_leds[rank_color]);
          state_leds[rank_color] = !state_leds[rank_color];
          msleep(delay_led[rank_color]);
      }
      gpio_set_value(array_leds[rank_color], 0); // shutdown leds when button==0 
      wait_event_interruptible(my_wq, button_pushed[rank_color] == 1);
  }
  return 0;
}



// ======================================================================================



static char *KEYS_NAME = "PB_KEY";
static ssize_t mon_int_b_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
{
  char kbuf[2000];
  size_t len;
  if(*ppos == 0){
    kbuf[0]='\0';
    sprintf(kbuf,"nb_buttons:%d\n",nb_devices_int_b);
    for(int i=0; i<nb_devices_int_b; ++i){
      sprintf(kbuf,"%sid:%d,cnt:%ld|",kbuf,irqs[i],irq_count[i]);
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
  .read = mon_int_b_read,
};

/* Interrupt handler */
static irqreturn_t hello_keys_isr(int irq, void *data)
{ 
    /*static bool value_red = 0;
    static bool value_green = 0;*/
    int i;
    struct device *dev = data;

    for(i=0;i<nb_devices_int_b;++i){
      if(irq == irqs[i]){
        /*
        ++irq_count[i];
        if(i<led_count){
          state_leds[i] = !state_leds[i];
          gpio_set_value(array_leds[i], state_leds[i]);
        }
        */

        ++irq_count[i];
        if(i<led_count){
          state_leds[i] = !state_leds[i];
          //led_state[i] = !led_state[i];
        }
        if(mutex_trylock(&lock_button[i])){
          button_pushed[i] = !button_pushed[i];
          mutex_unlock(&lock_button[i]);
        }
        // else can't lock: do nothing
        if(button_pushed[i]){
          wake_up_interruptible(&my_wq);
        }
        
      }
    }
    dev_info(dev, "interrupt received. key: %s, irq: %d\n", KEYS_NAME,irq);
    return IRQ_HANDLED;
}

static struct miscdevice mon_int_b_int_button_miscdevice = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "project_int_b_dev",
        .fops = &fops,
        .mode = 0666,
};

static int mon_int_b_probe(struct platform_device *pdev) {
    int ret_val,irq;

    struct device *dev = &pdev->dev;
    pr_info("mon_int_b_probe enter\n");
    dev_info(dev, "mon_int_b_probe() function is called.\n");

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
   
    mutex_init(&lock_button[nb_devices_int_b]);
    irqs[nb_devices_int_b++] = irq;
    dev_info(dev, "mon_int_b_probe() function is exited.\n");
   
    return 0;
}

static int  mon_int_b_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Remove called\n");
    return 0;
}

static const struct of_device_id mon_int_b_of_ids[] = {
        { .compatible = "training,intbutton"},
        {},
};

MODULE_DEVICE_TABLE(of, mon_int_b_of_ids);
static struct platform_driver mon_int_b_int_button_platform_driver = {
        .probe = mon_int_b_probe,
        .remove = mon_int_b_remove,
        .driver = {
                .name = "int_button",
                .of_match_table = mon_int_b_of_ids,
                .owner = THIS_MODULE,
        }
};




// =======================================================================================

static int my_led_probe(struct platform_device *pdev) {
    //int ret_val, irq;
    struct device *dev = &pdev->dev;
    char msg[64];

    pr_info("my_led_probe enter\n");
    dev_info(dev, "my_led_probe() function is called.\n");
    of_property_read_u32(pdev->dev.of_node,"id_led",&array_leds[led_count]);
    dev_info(dev," led %d \n",array_leds[led_count]);
    state_leds[led_count]=0;
    
    id_leds[led_count]=led_count;

    sprintf(msg,"clignote led gpio:%d  N°: %d",  array_leds[led_count], led_count);
    
    delay_led[led_count] = INIT_DELAY;
    tsk[led_count] = kthread_run(clignote_led,&id_leds[led_count],msg);


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

#define UPDATE_DELAY_MIN_MAX\
  do{\
    int k;\
    for(k=0; k<led_count; ++k){\
      if(delay_max<delay_led[k]) delay_max = delay_led[k];\
      if(delay_min>delay_led[k]) delay_min = delay_led[k];\
    }\
  }while(0);

static ssize_t my_led_dev_write(struct file *flip, const char *buf, size_t len, loff_t *off)
{
    char kbuffer[250];
    int ret;
    u8 i;
    long int delay_global;
    char cmd[20];
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
        //gpio_set_value(array_leds[i], 1);
        state_leds[i]=1;
        //if(mutex_trylock(&lock_button[i])){
          mutex_lock(&lock_button[i]);
          button_pushed[i] = 1;
          mutex_unlock(&lock_button[i]);
        //}
        //if(button_pushed[i])
        wake_up_interruptible(&my_wq);

        break;
      }
      sprintf(cmd,"off%d",i);
      if(strcmp(kbuffer,cmd) == 0){
        pr_info("led[%d] etteint\n",i);
        //gpio_set_value(array_leds[i], 0);
        state_leds[i]=0;
        //if(mutex_trylock(&lock_button[i])){
          mutex_lock(&lock_button[i]);
          button_pushed[i]=0;
          mutex_unlock(&lock_button[i]);
        //}
        wake_up_interruptible(&my_wq);
        break;
      }
      sprintf(cmd,"push%d",i);
      if(strcmp(kbuffer,cmd) == 0){
        state_leds[i] = !state_leds[i];
        pr_info("button pushed for led[%d], new state: %d\n",i,state_leds[i]);
        //gpio_set_value(array_leds[i], state_leds[i]);
       
          mutex_lock(&lock_button[i]);
          button_pushed[i] = !button_pushed[i];
          mutex_unlock(&lock_button[i]);

        if(button_pushed[i]){
          wake_up_interruptible(&my_wq);
        }
 
        break;
      }
      sprintf(cmd,"ledelay%d",i);
      pr_info("cmd: %s vs kbuf: %s size_cmd:%ld\n",cmd,kbuffer, strlen(cmd));
      if(strncmp(kbuffer,cmd,strlen(cmd)) == 0){
        delay_led[i]=parse_char_to_int(kbuffer, 10);
        pr_info("new delay_led[%d] : %ld\n",i,delay_led[i]);
        UPDATE_DELAY_MIN_MAX; 
        break;
      }
    }
    
      sprintf(cmd,"altern");
      if(strcmp(kbuffer,cmd) == 0){
        altern = !altern;
      }

      sprintf(cmd,"on");
      if(strcmp(kbuffer,cmd) == 0){
        for(i=0; i<led_count; ++i){
          pr_info("led[%d] allumé\n",i);
          //gpio_set_value(array_leds[i], 1);
          state_leds[i]=1;
          //if(mutex_trylock(&lock_button[i])){
            mutex_lock(&lock_button[i]);
            button_pushed[i] = 1;
            mutex_unlock(&lock_button[i]);
          //}
          //if(button_pushed[i])
          wake_up_interruptible(&my_wq);
          if(altern) msleep(delay_min /(led_count +1));
        }
      }
      else{
        sprintf(cmd,"off");
        if(strcmp(kbuffer,cmd) == 0){
          for(i=0; i<led_count; ++i){
            pr_info("led[%d] etteint\n",i);
            //gpio_set_value(array_leds[i], 0);
            state_leds[i]=0;
            //if(mutex_trylock(&lock_button[i])){
              mutex_lock(&lock_button[i]);
              button_pushed[i]=0;
              mutex_unlock(&lock_button[i]);
            //}
            wake_up_interruptible(&my_wq);
            if(altern) msleep(delay_min /(led_count +1));
          }
        }
        else{
          sprintf(cmd,"push");
          if(strcmp(kbuffer,cmd) == 0){
            for(i=0; i<led_count; ++i){
              state_leds[i] = !state_leds[i];
              pr_info("button pushed for led[%d], new state: %d\n",i,state_leds[i]);
              //gpio_set_value(array_leds[i], state_leds[i]);
                
                mutex_lock(&lock_button[i]);
                button_pushed[i] = !button_pushed[i];
                mutex_unlock(&lock_button[i]);

              if(button_pushed[i]){
                wake_up_interruptible(&my_wq);
              }
              if(altern) msleep(delay_min /(led_count +1));
            }
          }
          else{
            sprintf(cmd,"ledelay");
            pr_info("cmd: %s vs kbuf: %s size_cmd:%ld : res=%d\n",cmd,kbuffer, strlen(cmd), strncmp(kbuffer,cmd,strlen(cmd)));
            if(strncmp(kbuffer,cmd,strlen(cmd)) == 0){
              delay_global=parse_char_to_int(kbuffer, 10);
              for(i=0;i<led_count; ++i){
                delay_led[i]=delay_global;
                pr_info("new delay_led[%d] : %ld\n",i,delay_led[i]);
              }
              UPDATE_DELAY_MIN_MAX; 
            }
          }
        }
      }
      /*
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
      */
   // }
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

static ssize_t my_led_dev_read(struct file *flip, char *buf, size_t len, loff_t *off)
{
    char kbuf[250];
    size_t lenkbuf;
    u8 i;
    pr_info("Read operation led device: return led_count...\n");

    if(*off == 0){

      
      sprintf(kbuf,"led_count:%d",led_count);
      sprintf(kbuf,"%s|altern:%d",kbuf,altern);
      for(i=0;i<led_count;++i){
        sprintf(kbuf,"%s|ledelay%d:%ld",kbuf,i,delay_led[i]);          
      }
      lenkbuf = strlen(kbuf);
      if(copy_to_user(buf,kbuf,lenkbuf) != 0) // return nb caractère non copier, si 0 c'est reussi
        return -EFAULT;
      ++(*off);
      return lenkbuf;
    }
    return 0;
}


/* Declare a file_operations structure */
static const struct file_operations my_led_dev_fops = {
  .owner = THIS_MODULE,
  .write = my_led_dev_write,
  .read = my_led_dev_read,
};


MODULE_DEVICE_TABLE(of, my_led_of_ids);

static struct miscdevice my_leds_miscdevice = {
        .fops = &my_led_dev_fops,
        .minor = MISC_DYNAMIC_MINOR,
        .name = "project_leds", /*"mydev", nom dans /dev/ */
        .mode = 0666,
};


static struct platform_driver my_led_platform_driver = {
        .probe = my_led_probe,
        .remove = my_led_remove,
        .driver = {
                .name = "my_led_driver",
                .of_match_table = my_led_of_ids,
                .owner = THIS_MODULE,
        }
};


static int __init my_init(void) {
    int ret_val;
/* begin init leds  */
    ret_val = platform_driver_register(&my_led_platform_driver);
    if (ret_val !=0)
    {
        pr_err("platform led value returned %d\n", ret_val);
        return ret_val;
    }


    pr_info("Misc registering...\n");
    ret_val = misc_register(&my_leds_miscdevice);
    if (ret_val != 0) {
            pr_err("could not register the misc device mydev\n");
            return ret_val;
    }
/* end init leds */
   
    // ===============================================================

    /*begin init interupt button */
  
    pr_info("Push button driver enter\n");
    ret_val = platform_driver_register(&mon_int_b_int_button_platform_driver);
    if (ret_val !=0)
    {
        pr_err("platform value returned %d\n", ret_val);
        return ret_val;
    }
    ret_val = misc_register(&mon_int_b_int_button_miscdevice);
    if(ret_val != 0){
      pr_err("misc register value ret %d\n",ret_val);
      return ret_val;
    }


    /* end init interrupt button */


    return 0;
}

static void __exit my_exit(void) {
    int i;
     /* =======================================================
     * stop threads
     */
    for(i=0; i<led_count; ++i){
      kthread_stop(tsk[i]);
    }
    
     /* ===========================================================
      * begin exit leds  */
    pr_info("Deregister misc device led...\n");
    misc_deregister(&my_leds_miscdevice);
    platform_driver_unregister(&my_led_platform_driver);
    /* end exit leds */

   
    /* ====================================================================
     * begin exit interrupt button */
    pr_info("Push button driver exit\n");
    misc_deregister(&mon_int_b_int_button_miscdevice);
    platform_driver_unregister(&mon_int_b_int_button_platform_driver);
    /* end exit leds */
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("C.A. ABID");
MODULE_DESCRIPTION("Leds and interrupt button handler module");

