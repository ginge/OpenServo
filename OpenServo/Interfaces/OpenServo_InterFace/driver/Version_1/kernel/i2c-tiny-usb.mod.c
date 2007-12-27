#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x532fce98, "struct_module" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0xbe6d37a3, "i2c_add_adapter" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x53da3284, "usb_get_dev" },
	{ 0xde0bdcff, "memset" },
	{ 0xaa6016f0, "kmem_cache_alloc" },
	{ 0x1720ec8e, "malloc_sizes" },
	{ 0x5decf18d, "usb_register_driver" },
	{ 0xdd132261, "printk" },
	{ 0x933a4b34, "usb_control_msg" },
	{ 0xcde4a1eb, "i2c_del_adapter" },
	{ 0x37a0cba, "kfree" },
	{ 0x11bedbf3, "usb_put_dev" },
	{ 0x33e0d51a, "usb_deregister" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core,usbcore";

MODULE_ALIAS("usb:v0403pC631d*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "BD11532412E98B6351B1907");
