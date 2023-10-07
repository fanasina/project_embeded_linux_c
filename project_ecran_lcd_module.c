#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

/* Private device structure */
struct lcd_data {
    struct i2c_client *client;
    struct miscdevice lcd_miscdevice;
    const char * name ; 
};


static void lcd_send_data(struct i2c_client *client, u8 data);
static void lcd_set_position(struct i2c_client *client, int line, int col);

#define MAXCOL 16
#define MAXLIN 2

#define REFRESH_INIT 300

long int refresh[MAXLIN];

/*
 * each line handle
 */

void lcd_clear_line(struct i2c_client *client, u8 line){
  u8 j;
  dev_info(&client->dev,"lcd_clear_line %d\n", line);

  lcd_set_position(client,line,0);
  for(j=0; j<MAXCOL; ++j)
    lcd_send_data(client, ' ');

  lcd_set_position(client,line,0);
}

void lcd_print_MAX_on_line(struct i2c_client *client, char *buf, u8 line){
  u8 i;
  int len=strlen(buf);
  if(len > MAXCOL) len = MAXCOL;
  lcd_clear_line(client, line);
  for(i=0; i<len;++i){
    lcd_send_data(client, buf[i]);
  }
}

void lcd_print_all_on_line(struct i2c_client *client, char *buf, u8 line){
  int i, len;
  len=strlen(buf);
  if(len < MAXCOL){
    lcd_print_MAX_on_line(client, buf, line);
  }
  else{

    for(i=0;i < len - MAXCOL; ++i){
      lcd_print_MAX_on_line(client, buf+i, line);
      msleep(refresh[line]);
    }
  }
}

/*
 *   on all lines = screen
 */

void lcd_clear(struct i2c_client *client){
  u8 i,j;
  dev_info(&client->dev,"lcd_clear\n");
  for(i=0;i<MAXLIN;++i){
    lcd_set_position(client,i,0);
    for(j=0; j<MAXCOL; ++j)
      lcd_send_data(client, ' ');
  }
  lcd_set_position(client,0,0);
}


void lcd_print_MAX_on_screen(struct i2c_client *client, char *buf){
  u8 i;
  int len=strlen(buf);
  if(len > MAXCOL*MAXLIN) len = MAXCOL*MAXLIN;
  lcd_clear(client);
  for(i=0; i<len;++i){
    lcd_send_data(client, buf[i]);
    if(i==MAXCOL-1) lcd_set_position(client,1,0);
  }
}

void lcd_print_all_on_screen(struct i2c_client *client, char *buf){
  int i, len;
  len=strlen(buf);
  if(len < MAXCOL*MAXLIN){
    lcd_print_MAX_on_screen(client, buf);
  }
  else{

    for(i=0;i < len - MAXCOL*MAXLIN; ++i){
      lcd_print_MAX_on_screen(client, buf+i);
      msleep(refresh[0]);
    }
  }
}
/* 
* Write function 
*/
long int parse_field_char_to_lint(char *input, int field_to_extract){
   size_t len = strlen(input);
   long int val = 0, i;
   int cnt_colon=0;
   for(i=0; i< len && cnt_colon <= field_to_extract; ++i){
      if(input[i] == ':' ){
        if(cnt_colon == field_to_extract)
          return val;
        ++cnt_colon;
        val=0;
      }
      if(input[i] >='0' && input[i]<='9'){
        val *= 10;
        val += (input[i]-'0');
      }
   }
   return val;
}


static ssize_t lcd_write_file(struct file *file, const char __user *userbuf,size_t count, loff_t *ppos) {
    //int i;    
    char buf[100];
    struct lcd_data * lcd;
    lcd = container_of(file->private_data, struct lcd_data, lcd_miscdevice);    
    dev_info(&lcd->client->dev,"lcd_write_file entered on %s\n", lcd->name);
    if(copy_from_user(buf, userbuf, count)) {
        dev_err(&lcd->client->dev, "Bad copied value\n");
		return -EFAULT;
    }
    

    buf[count] = '\0'; /* replace \n with \0 */
   /* try to put on top of screen*/
    //lcd_set_position(lcd->client,0,0);
    //dev_info(&lcd->client->dev,"lcd_write_file entered on %s, try to goto 0,0\n", lcd->name);

    //lcd_clear(lcd->client);
      
    /* Convert the string to an unsigned long */
    /*
    for (i = 0; i < count - 1; i++){
      msleep(200);
      if( i%MAXCOL == 0){
        lcd_clear(lcd->client);
      }

      lcd_send_data(lcd->client, buf[i]);  
    }
    */

    //lcd_print_all_on_screen(lcd->client,buf);
    //for(i=0;i<3;++i) 
    
    if(buf[0]=='0'){  
      lcd_print_all_on_line(lcd->client,buf+1,0);
    }
    else if(buf[0]=='1'){
      lcd_print_all_on_line(lcd->client,buf+1,1);
    }
    else if(strncmp(buf,"refresh0",8) == 0){
      refresh[0]=parse_field_char_to_lint(buf,1);
    }
     else if(strncmp(buf,"refreshr1",8) == 0){
      refresh[1]=parse_field_char_to_lint(buf,1);
    }
     else if(strncmp(buf,"refresh",7) == 0){
      refresh[0]=parse_field_char_to_lint(buf,1);
      refresh[1]=parse_field_char_to_lint(buf,1);
    }
    else{
      lcd_print_all_on_screen(lcd->client, buf);
    }
    return count;
}

static const struct file_operations lcd_fops = {
    .owner = THIS_MODULE,
    .write = lcd_write_file,
};
/*
*/
static void lcd_init(struct i2c_client *client);

/* The probe() function is called after binding */
static int lcd_probe(struct i2c_client * client,const struct i2c_device_id * id) {    
    struct lcd_data * lcd;
    dev_info(&client->dev,"Probe entered...\n");
    /* Allocate a private structure */
    lcd = devm_kzalloc(&client->dev, sizeof(struct lcd_data), GFP_KERNEL);
    /* Store pointer to the device-structure in bus device context */
    i2c_set_clientdata(client,lcd);
    /* Store pointer to the I2C client device in the private structure */
    lcd->client = client;
    of_property_read_string(client->dev.of_node, "label", &lcd->name);    
    lcd->lcd_miscdevice.name = lcd->name;
    lcd->lcd_miscdevice.minor = MISC_DYNAMIC_MINOR;
    lcd->lcd_miscdevice.fops = &lcd_fops;
    /* to write as user  */
    lcd->lcd_miscdevice.mode = 0666;
    /* Register misc device */
    misc_register(&lcd->lcd_miscdevice);        
    /* Initialize the LCD chip */
    lcd_init(client);
    return 0;
}

static void lcd_remove(struct i2c_client * client)
{
    struct lcd_data * lcd;    
    /* Get device structure from bus device context */
    lcd = i2c_get_clientdata(client);    
    dev_info(&client->dev,"lcd_remove is entered on %s\n", lcd->name);
    /* Deregister misc device */
    misc_deregister(&lcd->lcd_miscdevice);    
}

static const struct of_device_id lcd_dt_ids[] = {
    {.compatible = "training,lcd2x16", },
    {}
};
MODULE_DEVICE_TABLE(of, lcd_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
    {.name = "lcd2x16", },
    {}
};

MODULE_DEVICE_TABLE(i2c, i2c_ids);
static struct i2c_driver lcd_driver = {
    .driver = {
        .name = "lcd2x16",
        .owner = THIS_MODULE,
        .of_match_table = lcd_dt_ids,
    },
    .probe = lcd_probe,
    .remove = lcd_remove,
    .id_table = i2c_ids,
};
/***************************************************************************/
//##################################################################################################
#define LCD_REG_MAN_ID			0x27

//##################################################################################################
#define LCD_BL					0x08  // backlightFlag
#define LCD_EN					0x04  // Enable bit
#define LCD_RW					0x02  // Read/Write bit
#define LCD_RS					0x01  // Register select bit
#define LCD_MASK_DISABLE_EN		0xFB  // MASK of control

//##################################################################################################
//# LCD Commands:
//##################################################################################################
#define LCD_CMD_CLEARDISPLAY    0x01
#define LCD_CMD_RETURNHOME      0x02
#define LCD_CMD_ENTRYMODESET    0x04
#define LCD_CMD_DISPLAYCONTROL  0x08
#define LCD_CMD_CURSORSHIFT     0x10
#define LCD_CMD_FUNCTIONSET     0x20
#define LCD_CMD_SETCGRAMADDR    0x40
#define LCD_CMD_SETDDRAMADDR    0x80
#define LCD_CLEARDISPLAY    	0x01
#define LCD_RETURNHOME      	0x02
#define LCD_ENTRYMODESET    	0x04
#define LCD_DISPLAYCONTROL  	0x08
#define LCD_CURSORSHIFT     	0x10
#define LCD_FUNCTIONSET     	0x20
#define LCD_SETCGRAMADDR    	0x40
#define LCD_CTRL_BLINK_ON	0x01
#define LCD_CTRL_BLINK_OFF	0x00
#define LCD_CTRL_DISPLAY_ON	0x04
#define LCD_CTRL_DISPLAY_OFF	0x00
#define LCD_CTRL_CURSOR_ON	0x02
#define LCD_CTRL_CURSOR_OFF	0x00
#define LCD_DISPLAYON   	0x04
#define LCD_DISPLAYOFF  	0x00
#define LCD_CURSORON    	0x02
#define LCD_CURSOROFF   	0x00
#define LCD_BLINKON     	0x01
#define LCD_BLINKOFF    	0x00
#define LCD_ENTRYRIGHT           0x00
#define LCD_ENTRYLEFT            0x02
#define LCD_ENTRYSHIFTINCREMENT  0x01
#define LCD_ENTRYSHIFTDECREMENT  0x00
#define LCD_DISPLAYMOVE  	0x08
#define LCD_CURSORMOVE   	0x00
#define LCD_MOVERIGHT    	0x04
#define LCD_MOVELEFT     	0x00
#define LCD_8BITMODE  		0x10
#define LCD_4BITMODE  		0x00
#define LCD_2LINE     		0x08
#define LCD_1LINE     		0x00
#define LCD_5x10DOTS  		0x04
#define LCD_5x8DOTS   		0x00
//##################################################################################################
/**   Function:	static int lcd_i2c_write_byte(struct i2c_client *client, u8 *data)
 **   Desc:	This function will go write a byte in i2c protocol.
 */
static int lcd_i2c_write_byte(struct i2c_client *client, u8 *data) {
  int ret;
  ret = i2c_master_send(client, data, 1);
  if (ret < 0)
    dev_warn(&client->dev, "Write byte in i2c ['0x%02X'] failed.\n", ((int)*data));
  return ret;
}
/*
* Read data
*/
static int lcd_i2c_read_byte(struct i2c_client *client) {
  u8 i2c_data[1];
  int ret = 0;
  ret = i2c_master_recv(client, i2c_data, 1);
  if (ret < 0) {
    dev_warn(&client->dev, "i2c read data failed\n");
    return ret;
  }
  return (i2c_data[0]);
}
/*
*/
bool backlightFlag = true;

static void lcd_en_strobe(struct i2c_client *client) {  
  u8 lcd_data;
  lcd_data = lcd_i2c_read_byte(client);
  lcd_data = (lcd_data | LCD_EN | (backlightFlag == 1 ? LCD_BL : 0x00));
  lcd_i2c_write_byte(client, &lcd_data);
  ndelay(1);	
  lcd_data &= LCD_MASK_DISABLE_EN | (backlightFlag == 1 ? LCD_BL : 0x00);
  lcd_i2c_write_byte(client, &lcd_data);
}

static void lcd_write_4bits(struct i2c_client *client, u8 value)  {  
  u8 data[1];
  data[0]=value | LCD_EN;
  lcd_i2c_write_byte(client,data);
  msleep(1);
  //lcd_en_strobe(client);
  data[0]=(u8)(value & (u8)~LCD_EN);
  lcd_i2c_write_byte(client,data);
  msleep(50);
  //lcd_en_strobe(client);
}

static void lcd_send_byte(struct i2c_client *client, u8 msb, u8 lsb)  {
    if (backlightFlag) {
        lsb = lsb | LCD_BL;
        msb = msb | LCD_BL;
    }
    lcd_write_4bits(client,msb);
    lcd_write_4bits(client,lsb);
}

/*
* Send command
*/
static void  lcd_send_cmd(struct i2c_client *client, u8 cmd) {        
    u8 MSb = cmd  & 0xF0;
    u8 LSb = (cmd & 0x0F) << 4;
    lcd_send_byte(client,MSb, LSb);
}

/*
* Set cursor position
*/
static void lcd_set_position(struct i2c_client *client, int line, int col)
{
  u8 lcd_row_offset[4] = {0x80, 0xC0, 0x14, 0x54};
  lcd_send_cmd(client,  (lcd_row_offset[line] + col)); 
}

/**    
 **   Desc:
 */
static void lcd_send_data(struct i2c_client *client, u8 data)
{
  u8 d;   
  d = (data & 0xF0) | LCD_RS | (backlightFlag == 1 ? LCD_BL : 0x00);
  lcd_i2c_write_byte(client, &d);
  lcd_en_strobe(client);
  d = (data << 4) | LCD_RS | (backlightFlag == 1 ? LCD_BL : 0x00);
  lcd_i2c_write_byte(client, &d);
  lcd_en_strobe(client);   
}

/*
* Display a string  
*/

static void lcd_puts(struct i2c_client *client, const char *string, u8 line, u8 col, u8 count)
{
  u8 i;
  lcd_set_position(client, line, col);
  for (i = 0; i < count; i++)      
    lcd_send_data(client, string[i]);  
}

/*
* Initialize the lcd
*/
static void lcd_init(struct i2c_client *client)
{
  u8 msg[] = {"PROJECT POEI"};
  u8 i;
  int displayshift = (LCD_CURSORMOVE | LCD_MOVERIGHT);
  int displaymode = (LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
  int displaycontrol = (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
  // Initialization
  lcd_send_cmd(client, 0x02);
  lcd_send_cmd(client, LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  lcd_send_cmd(client, LCD_DISPLAYCONTROL | displaycontrol);
  lcd_send_cmd(client, LCD_ENTRYMODESET | displaymode);
  lcd_send_cmd(client, LCD_CLEARDISPLAY);
  lcd_send_cmd(client, LCD_CURSORSHIFT | displayshift);  
  lcd_send_cmd(client, LCD_RETURNHOME);
  lcd_puts(client, msg, 0, 0,strlen(msg));  
  
  for(i=0;i<MAXLIN;++i)
    refresh[i] = REFRESH_INIT;
   
  //lcd_print_on_all_screen(client,msg);
}
module_i2c_driver(lcd_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("C.A. ABID");
MODULE_DESCRIPTION("This is a client i2c driver that control a LCD display connected via i2c bus");
