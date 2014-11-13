#include<linux/module.h>
#include<linux/kernel.h>
 
MODULE_LICENSE("GPL");

void initialize(void)
{
}

void finalize(void)
{
}
 
static int pslist_init(void)
{
    initialize();
}
 
static void pslist_exit(void)
{
    finalize();
    printk("###################################################################\n");
}
 
module_init(pslist_init);
module_exit(pslist_exit);
