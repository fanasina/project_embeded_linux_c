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
	{ 0x9ecbb978, "i2c_register_driver" },
	{ 0x7860c8aa, "_dev_info" },
	{ 0x8721b4ea, "misc_deregister" },
	{ 0xd084e52a, "i2c_del_driver" },
	{ 0x61e7cbf, "i2c_transfer_buffer_flags" },
	{ 0xb50044cb, "_dev_warn" },
	{ 0xf9a482f9, "msleep" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x54a4f7cf, "devm_kmalloc" },
	{ 0x6b09b338, "of_property_read_string" },
	{ 0xf6dbc429, "misc_register" },
	{ 0x98cf60b3, "strlen" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xdcb764ad, "memset" },
	{ 0xb2b01c3c, "_dev_err" },
	{ 0x5a921311, "strncmp" },
	{ 0x87b40f76, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:lcd2x16");
MODULE_ALIAS("of:N*T*Ctraining,lcd2x16");
MODULE_ALIAS("of:N*T*Ctraining,lcd2x16C*");

MODULE_INFO(srcversion, "7871A831A337FE9D78A6EAA");
