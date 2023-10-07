#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x5c6615fa, "__platform_driver_register" },
	{ 0x92997ed8, "_printk" },
	{ 0xf6dbc429, "misc_register" },
	{ 0xfe990052, "gpio_free" },
	{ 0x8721b4ea, "misc_deregister" },
	{ 0xed22970c, "platform_driver_unregister" },
	{ 0x7860c8aa, "_dev_info" },
	{ 0xd1894777, "of_property_read_variable_u32_array" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x3953e167, "gpio_to_desc" },
	{ 0xbd22021a, "gpiod_direction_output_raw" },
	{ 0xf9a482f9, "msleep" },
	{ 0x220e5d45, "gpiod_set_raw_value" },
	{ 0xd5d8019a, "gpiod_direction_input" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xf9b14f48, "gpiod_get_raw_value" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x98cf60b3, "strlen" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0x87b40f76, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Ctraining,dht11platform");
MODULE_ALIAS("of:N*T*Ctraining,dht11platformC*");

MODULE_INFO(srcversion, "1D85A927BA7664A26DEC0AB");
