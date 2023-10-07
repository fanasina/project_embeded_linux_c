obj-m := project_dth11_capteur.o project_ecran_lcd_module.o project_led_and_int_button.o 
#project_DiodesLedDTs.o project_server_led_module.o project_push_button_interrupt_led.o

# emplacement du souce linux compilé sur le pc
KERNEL_DIR ?= $(HOME)/progr_/linux/__rpi3b64/linux

# rien à changer pour all et clean

all:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean
deploy:
	# scp *.ko  user_raspberry@ip_raspery:/home/user_raspberry
	scp *.ko fanasina@192.168.75.140:/home/fanasina/modules/project/
