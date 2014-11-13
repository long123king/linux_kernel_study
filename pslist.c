#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
 
MODULE_LICENSE("GPL");

void raw_print(unsigned long size, const u_char* data)
{
    if (size == 0)
    {
        return;
    }
 
    unsigned long i = 0;
    for (i=0;i<16;i++)
    {
        printk("%4X", i);
    }
    printk("\n");
    for (i=0;i<16;i++)
    {
        printk("%4s", "__");
    }
 
    char lineSummary[17] = {0,};
    unsigned long pos = 0;
    for (i=0;i<size;i++)
    {
        if ((pos = i % 16) == 0)
        {
            if (i != 0)
            {
                printk(" ---- %s\n", lineSummary);
                memset(lineSummary, 0, 17);
            }
            else
            {
                printk("\n");
            }        
        }
 
        printk("  %02X", *(data + i));
 
        if (*(data + i) >= 0x20 && *(data + i) <= 0x7E)
        {
            lineSummary[pos] = *(data + i);
        }
        else
        {
            lineSummary[pos] = ' ';
        }
    }
 
    if (size % 16 != 0)
    {
        for (i=0;i<16 - (size%16);i++)
        {
            printk("    ");
        }
    }
 
    printk(" ---- %s\n", lineSummary);
    printk("\n");    
}

void dumpPageDirectoryEntry(u32 entry)
{
    u8 present;
    u8 read_write;
    u8 user_supervisor;
    u8 accessed;
    u8 dirty;
 
    u32 pageTableAddr;
 
    present = entry & 0x00000001;
    read_write = (entry & 0x00000002) >> 1;
    user_supervisor = (entry & 0x00000004) >> 2;
    accessed = (entry & 0x00000020) >> 5;
    dirty = (entry & 0x00000040) >> 6;
 
    pageTableAddr = entry & 0xFFFFF000;

    if (present == 1)
    {        
        printk("[0x%08X] --> [0x%08X] ", entry, pageTableAddr);
        if (read_write == 1)
        {
            printk("W ");
        }
        else
        {
            printk("R ");
        }
 
        if (user_supervisor  == 1)
        {
            printk("U ");
        }
        else
        {
            printk("S ");
        }
 
        if (accessed == 1)
        {
            printk("A ");
        }
        else
        {
            printk("  ");
        }
 
        if (dirty)
        {
            printk("D ");
        }
        else
        {
            printk("  ");
        }
        
    }
    else
    {
        printk("");
    }
 
    printk("\n");
 
}
 
void dumpPageDirectory(u32 size, u32* entries)
{
    u32 i;
 
    printk("%12s --> %s \n", "pgdir entry", "pgtable");
    for (i = 0; i < size/4; ++i)
    {
	if (*(entries+i) != 0)
        {
            dumpPageDirectoryEntry(*(entries + i));
        }
    }
}

void analyzeTaskPgd(void)
{
    struct task_struct *pos;
    struct list_head *current_head;
 
    current_head=&(current->tasks);
    if (current_head == NULL)
        return 0;

    //printk("%10s   %12s %s\n", "pid", "page dir", "process name");
    list_for_each_entry(pos,current_head,tasks)
    {
         if (pos == NULL || pos->mm == NULL)
             continue;
         
         //printk("[%8d] : [0x%08X] %s\n",pos->pid, pos->mm->pgd, pos->comm);
  
         if (strcmp(pos->comm, "mate-terminal") == 0)
         {
             printk("[%8d] : [0x%08X] %s\n",pos->pid, pos->mm->pgd, pos->comm);
             raw_print(64, pos->mm->pgd);
             dumpPageDirectory(1 << PAGE_SHIFT, pos->mm->pgd);
         }
    }
}

void initialize(void)
{
    analyzeTaskPgd();
}

void finalize(void)
{
}
 
static int pslist_init(void)
{
    printk("###################################################################\n");
    initialize();
}
 
static void pslist_exit(void)
{
    finalize();
}
 
module_init(pslist_init);
module_exit(pslist_exit);
