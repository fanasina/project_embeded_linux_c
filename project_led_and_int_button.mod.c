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
	{ 0x7860c8aa, "_dev_info" },
	{ 0xbb9ed3bf, "mutex_trylock" },
	{ 0xe2964344, "__wake_up" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x73253b68, "platform_get_irq" },
	{ 0xb2b01c3c, "_dev_err" },
	{ 0x4e98f6ee, "devm_request_threaded_irq" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xb605dee7, "kthread_stop" },
	{ 0x8721b4ea, "misc_deregister" },
	{ 0xed22970c, "platform_driver_unregister" },
	{ 0xd1894777, "of_property_read_variable_u32_array" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xb6310c62, "kthread_create_on_node" },
	{ 0x7e71d581, "wake_up_process" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x3953e167, "gpio_to_desc" },
	{ 0x220e5d45, "gpiod_set_raw_value" },
	{ 0xf9a482f9, "msleep" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x98cf60b3, "strlen" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xdcb764ad, "memset" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x5a921311, "strncmp" },
	{ 0xe914e41e, "strcpy" },
	{ 0x87b40f76, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Ctraining,ledplatform");
MODULE_ALIAS("of:N*T*Ctraining,ledplatformC*");
MODULE_ALIAS("of:N*T*Ctraining,intbutton");
MODULE_ALIAS("of:N*T*Ctraining,intbuttonC*");

MODULE_INFO(srcversion, "0A60E109BB1A6FAA3167156");
