#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/mm_types.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <asm/desc.h>
#include <linux/smp.h>
#include <asm/msr.h>
#include <asm-generic/percpu.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");

struct tss_segment_32 {
	u32 prev_task_link;
	u32 esp0;
	u32 ss0;
	u32 esp1;
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;
	u32 cs;
	u32 ss;
	u32 ds;
	u32 fs;
	u32 gs;
	u32 ldt_selector;
	u16 t;
	u16 io_map;
};

void printBinary(char* name, char* buffer, unsigned long size)
{
	int i;

	printk("%s\n", name);
	for (i = 0; i < size; ++i)
	{
		printk("    %02X", *(buffer + i));
		if (i % 16 == 0)
		{
			printk("\n");
		}
	}
	if (size % 16 != 0)
	{
		printk("\n");
	}
}

void printRawData(unsigned long size, const u_char* data)
{
	if (size == 0)
	{
		return;
	}

	printk("Memory at 0x%08x\n", (u32)data);
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

void printBuddySystemInfo(struct free_area* free_area)
{
	int j;
	struct free_area* pfreearea = free_area;
	if (pfreearea == NULL)
		return;
	for (j = 0; j < MAX_ORDER; ++j)
	{
		//printBinary("free_area", (char*)(pfreearea + j), sizeof(struct free_area));
		// printRawData(sizeof(struct free_area), (char*)(pfreearea + j));
		printk("%5d", (pfreearea + j)->nr_free);	
	}
	printk("\n");
}

void printPagesetInfo(struct per_cpu_pageset *pageset)
{
	printk("pageset: 0x%08x\n", pageset);
	printRawData(sizeof(*pageset), pageset);
	// struct per_cpu_pages pages = pageset->pcp;
	// printk("Pageset Info:\n");

	// printk("    count:\t\t%d\n", pages.count);
	// printk("    high:\t\t%d\n", pages.high);
	// printk("    batch:\t\t%d\n", pages.batch);
}

int analyzeUMANode()
{
	int i,j;
	unsigned long node_present_pages;
	unsigned long node_spanned_pages;
	struct pglist_data* node_0;
	
	struct zone* pzone;
	unsigned long start_pfn;
	unsigned long present_pages;
	unsigned long spanned_pages;
	
	// struct page* first_page;


	printk("###################################################################\n");

	// printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	node_0 = NODE_DATA(0);
	printk("node_0 at 0x%08x\n", node_0);
	if (node_0 == NULL)
		return 0;

	node_present_pages = node_0->node_present_pages;
	node_spanned_pages = node_0->node_spanned_pages;
	printk("present pages: %d\n", node_present_pages);
	printk("spanned pages: %d\n", node_spanned_pages);
	
	for (i = 0; i < MAX_NR_ZONES; ++i)
	{
		pzone = &node_0->node_zones[i];
		if (pzone == NULL)
			continue;
		printk("Zone %d Name: %s\n", i, pzone->name);
		start_pfn = pzone->zone_start_pfn;
		printk("start_pfn : %d\n", start_pfn);
		present_pages = pzone->present_pages;
		printk("present_pages : %d\n", present_pages);
		spanned_pages = pzone->spanned_pages;
		printk("spanned_pages : %d\n", spanned_pages);
		printk("%8d MB - %8d MB\n", start_pfn * PAGE_SIZE / 1024 / 1024, (start_pfn + spanned_pages) * PAGE_SIZE / 1024 / 1024);
		
		printBuddySystemInfo(&pzone->free_area);
		printPagesetInfo(pzone->pageset);

	}
	// first_page = mem_map;
	// int i;
	// for (i = 0; i < node_present_pages; ++i)
	// {
	// 	unsigned long bLocked = (first_page + i)->flags == PG_locked;
	// 	if (!bLocked)
	// 	{
	// 		// printk("Page->virtual : 0x%08x, flags = 0x%08x\n", page_address(first_page + i), (first_page + i)->flags);
	// 	}
	// }
	return 0;
}

int analyzeProcesses()
{
	struct task_struct *pos;
    struct list_head *current_head;
	int count=0;

	struct vm_area_struct* vma;
	bool bFirst;
	struct file* vm_file;
	char szBuff[1024] = {0};
	char szTmp[1024] = {0};

	struct dentry* vm_dentry;
	struct dentry* parent;

	unsigned long vm_flags;
	    
	printk("Traversal module is working..\n");
	    // printk("Parameters:\n");
	    // printk("%20s\t0x%08x\n", "buffer", buffer);
	    // printk("%20s\t0x%08x\n", "buffer_pos", buffer_pos);
	    // printk("%20s\t0x%08x\n", "offset", offset);
	    // printk("%20s\t0x%08x\n", "buffer_len", buffer_len);
	    // printk("%20s\t0x%08x\n", "eof", eof);
	    // printk("%20s\t0x%08x\n", "data", data);

	current_head=&(current->tasks);
	if (current_head == NULL)
		return 0;
	printk("%6s\t%6s\t%s\n", "tgid", "pid", "name");
	list_for_each_entry(pos,current_head,tasks)
	{
	    count++;
	    if (pos == NULL)
        	continue;
	    // printk("%6d\t%6d\t%s\n", pos->tgid, pos->pid, pos->comm);
	    // continue;
	    if (strcmp(pos->comm, "gnome-terminal") != 0)
	    {
	        continue;
	    }
        // ret += sprintf(buffer+ret, "[process %d]: %s\'s pid is %d\n",count,pos->comm,pos->pid);
        // ret += sprintf(buffer + ret, "%40s:\n", pos->comm);
        // ret += sprintf(buffer + ret, "\t%20s:\t%d\n", "pid", pos->pid);
        // ret += sprintf(buffer + ret, "\t%20s:\t%d\n", "state", pos->state);
        // ret += sprintf(buffer + ret, "\t%20s:\t%ll\n", "exec_start", pos->se.exec_start);
        // ret += sprintf(buffer + ret, "\t%20s:\t%ll\n", "sum_exec_runtime", pos->se.sum_exec_runtime);
        // ret += sprintf(buffer + ret, "\t%20s:\t%ll\n", "vruntime", pos->se.vruntime);
        // ret += sprintf(buffer + ret, "\t%20s:\t%ll\n", "prev_sum_exec_runtime", pos->se.prev_sum_exec_runtime);
        // printk("%40s:\n", pos->comm);
        // printk("\t%20s:\t%d\n", "pid", pos->pid);
        // printk("\t%20s:\t%d\n", "state", pos->state);
        // printk("\t%20s:\t0x%016llx\n", "exec_start", pos->se.exec_start);
        // printk("\t%20s:\t0x%016llx\n", "sum_exec_runtime", pos->se.sum_exec_runtime);
        // printk("\t%20s:\t0x%016llx\n", "vruntime", pos->se.vruntime);
        // printk("\t%20s:\t0x%016llx\n", "prev_sum_exec_runtime", pos->se.prev_sum_exec_runtime);
        
        if (pos->mm != NULL && pos->mm->mmap != NULL)
       	{
	        // printk("[process %d]: mmap is 0x%08x\n", pos->pid, pos->mm->mmap);
	        vma = pos->mm->mmap;
	        bFirst = true;
	        while (vma != NULL && (vma != pos->mm->mmap || bFirst))
	        {
	        	printk("[0x%08x - 0x%08x ]: ", vma->vm_start, vma->vm_end);	

	        	printk("[0x%08x Pages] ", (vma->vm_end - vma->vm_start) >> PAGE_SHIFT);

				vm_flags = vma->vm_flags;
		        printk("0x%08x [", vm_flags);
		        // if ((vm_flags & VM_GROWSUP) != 0)
		        // {
		        // 	printk("GU ");
		        // }
		        // else
		        // {
		        // 	printk("   ");
		        // }
		        // if ((vm_flags & VM_GROWSDOWN) != 0)
		        // {
		        // 	printk("GD ");
		        // }
		        // else
		        // {
		        // 	printk("   ");
		        // }
		        if ((vm_flags & VM_READ) != 0)
		        {
		        	printk("R ");
		        }
		        else
		        {
		        	printk("  ");
		        }
		        if ((vm_flags & VM_WRITE) != 0)
		        {
		        	printk("W ");
		        }
		        else
		        {
		        	printk("  ");
		        }
		        if ((vm_flags & VM_EXEC) != 0)
		        {
		        	printk("E ");
		        }
		        else
		        {
		        	printk("  ");
		        }
		        if ((vm_flags & VM_SHARED) != 0)
		        {
		        	printk("S");
		        }
		        else
		        {
		        	printk(" ");
		        }
		        printk("]");

	        	vm_file = vma->vm_file;
	        	if (vm_file != NULL)
	        	{    			        	
	        		vm_dentry = vm_file->f_dentry;
	        		if (vm_dentry != NULL)
	        		{
	        			// parent = vm_dentry;
	        			// while (parent != NULL && strcmp(parent->d_name.name, "/") != 0)
	        			// {
	        			// 	//printk("\t%20s:\t%s\n", "name", parent->d_name.name);
	        			// 	strcpy(szTmp, szBuff);
	        			// 	memset(szBuff, 0, 1024);
	        			// 	sprintf(szBuff, "%s/%s", parent->d_name.name, szTmp);
	        			// 	parent = parent->d_parent;
	        			// }
	        			// szBuff[strlen(szBuff) - 1] = 0;
	        			printk("\t/%s", vm_dentry->d_name.name);
	        		}
		        	
		        	
	        	}
	        	printk("\n");
	        	bFirst = false;
	        	vma = vma->vm_next;
	        }
       	}	
    }

	return 0;	/* everything is ok */
}

void dumpTSS(struct tss_segment_32* tss)
{
	printk("TSS:\n");
	printk("\t%16s : 0x%08X\n", "prev_task_link", tss->prev_task_link);
	printk("\t%16s : 0x%08X\n", "esp0",tss->esp0);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ss0", tss->ss0, tss->ss0 >> 3, tss->ss0&0x0004, tss->ss0&0x0003);
	printk("\t%16s : 0x%08X\n", "esp1",tss->esp1);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ss1", tss->ss1, tss->ss1 >> 3, tss->ss1&0x0004, tss->ss1&0x0003);
	printk("\t%16s : 0x%08X\n", "esp2",tss->esp2);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ss2", tss->ss2, tss->ss2 >> 3, tss->ss2&0x0004, tss->ss2&0x0003);
	printk("\t%16s : 0x%08X\n", "cr3",tss->cr3);
	printk("\t%16s : 0x%08X\n", "eip",tss->eip);
	printk("\t%16s : 0x%08X\n", "eflags",tss->eflags);
	printk("\t%16s : 0x%08X\n", "eax",tss->eax);
	printk("\t%16s : 0x%08X\n", "ecx",tss->ecx);
	printk("\t%16s : 0x%08X\n", "edx",tss->edx);
	printk("\t%16s : 0x%08X\n", "ebx",tss->ebx);
	printk("\t%16s : 0x%08X\n", "esp",tss->esp);
	printk("\t%16s : 0x%08X\n", "ebp",tss->ebp);
	printk("\t%16s : 0x%08X\n", "esi",tss->esi);
	printk("\t%16s : 0x%08X\n", "edi",tss->edi);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "es", tss->es, tss->es >> 3, tss->es&0x0004, tss->es&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "cs", tss->cs, tss->cs >> 3, tss->cs&0x0004, tss->cs&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ss", tss->ss, tss->ss >> 3, tss->ss&0x0004, tss->ss&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ds", tss->ds, tss->ds >> 3, tss->ds&0x0004, tss->ds&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "fs", tss->fs, tss->fs >> 3, tss->fs&0x0004, tss->fs&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "gs", tss->gs, tss->gs >> 3, tss->gs&0x0004, tss->gs&0x0003);
	printk("\t%16s : 0x%08X[index=%04X, TI=%d, RPL=%d]\n", "ldt_selector", tss->ldt_selector, tss->ldt_selector >> 3, tss->ldt_selector&0x0004, tss->ldt_selector&0x0003);
	printk("\t%16s : 0x%04X\n", "t",tss->t);
	printk("\t%16s : 0x%04X\n", "io_map",tss->io_map);
	// printRawData(0x40, tss + tss->io_map);
	printk("\n");
}

void analyzeGDTEntry(char* buffer)
{
	u32 limit;
	u32 base;
	u8 type;
	u8 dpl;
	u8 granularity;
	u8 systemFlag;
	u8 present;
	u8 dbFlag;

	granularity = ((*(u8*)(buffer + 6)) & 0x80) >> 7;
	dbFlag = ((*(u8*)(buffer + 6)) & 0x40) >> 6;

	present = ((*(u8*)(buffer + 5)) & 0x80) >> 7;
	systemFlag = ((*(u8*)(buffer + 5)) & 0x10) >> 4;

	dpl = ((*(u8*)(buffer + 5)) & 0x60) >> 5;
	type = ((*(u8*)(buffer + 5)) & 0x0F);

	limit = *(u16*)buffer;
	limit += ((*(u8*)(buffer + 6)) & 0x0F) << 16;

	if (granularity == 1)
	{
		limit = ((limit + 1) << 12) - 1;
	}

	base = 0;
	base += *(u16*)(buffer + 2);
	base += (*(u8*)(buffer + 4)) << 16;
	base += (*(u8*)(buffer + 7)) << 24;

	if (limit == 0)
	{
		printk("[null]\n");
	}
	else
	{
		printk("0x%08x : 0x%08x, ", base, base + limit);
		printk("G[%d] ", granularity);
		printk("D/B[%d] ", dbFlag);
		printk("P[%d] ", present);
		printk("DPL[%d] ", dpl);
		printk("S[%d] ", systemFlag);
		printk("Type[%2d] ", type);
		if (systemFlag == 1)
		{
			if ((type & 0x08) != 0 )
			{
				// Code
				printk("Code[");
				if ((type & 0x04) != 0)
				{
					printk("Conforming ");
				}
				if ((type & 0x02) != 0)
				{
					printk("Read-Enable ");
				}
				if ((type & 0x01) != 0)
				{
					printk("Accessed ");
				}
				printk("]");
			}
			else
			{
				// Data
				printk("Data[");
				if ((type & 0x04) != 0)
				{
					printk("Expand-Down ");
				}
				if ((type & 0x02) != 0)
				{
					printk("Write-Enable ");
				}
				if ((type & 0x01) != 0)
				{
					printk("Accessed ");
				}
				printk("]");
			}
		}
		else
		{
			if (type == 0x02)
			{
				printk("[LDT]");
			}
			else if (type == 0x05)
			{
				printk("[Task Gate]");
			}
			else if (type == 0x09)
			{
				printk("[32-Bit TSS(Available)]");
				struct tss_segment_32 tss;
				memcpy(&tss, base, sizeof(tss));
				dumpTSS(&tss);
			}
			else if (type == 0x0B)
			{
				printk("[32-Bit TSS(Busy)]");
				struct tss_segment_32 tss;
				memcpy(&tss, base, sizeof(tss));
				dumpTSS(&tss);
			}			
			else if (type == 0x0C)
			{
				printk("[32-Bit Call Gate]");
			}
			else if (type == 0x0E)
			{
				printk("[32-Bit Interrupt Gate]");
			}
			else if (type == 0x0F)
			{
				printk("[32-Bit Trap Gate]");
			}
		}
		
		printk("\n");
	}


}

void analyzeTLS(char* buffer)
{
	u32 limit;
	u32 base;
	u8 type;
	u8 dpl;
	u8 granularity;
	u8 systemFlag;
	u8 present;
	u8 dbFlag;

	granularity = ((*(u8*)(buffer + 6)) & 0x80) >> 7;
	dbFlag = ((*(u8*)(buffer + 6)) & 0x40) >> 6;

	present = ((*(u8*)(buffer + 5)) & 0x80) >> 7;
	systemFlag = ((*(u8*)(buffer + 5)) & 0x10) >> 4;

	dpl = ((*(u8*)(buffer + 5)) & 0x60) >> 5;
	type = ((*(u8*)(buffer + 5)) & 0x0F);

	limit = *(u16*)buffer;
	limit += ((*(u8*)(buffer + 6)) & 0x0F) << 16;

	if (granularity == 1)
	{
		limit = ((limit + 1) << 12) - 1;
	}

	

	base = 0;
	base += *(u16*)(buffer + 2);
	base += (*(u8*)(buffer + 4)) << 16;
	base += (*(u8*)(buffer + 7)) << 24;

	printk("0x%08x : 0x%08x, ", base, base + limit);

	printk("TLS:%x\n", limit);
	printRawData(0x100, (char*)base);
}

void analyzeGDT(u32 size, char* buffer)
{
	int i;
	for (i = 0; i < size; i += 8)
	{
		printk("[%4X] ", i/8);		
		analyzeGDTEntry(buffer + i);
	}

	// analyzeTLS(buffer + 6*8);
}

void analyzeIDTEntry(char* buffer)
{
	u32 offset;
	u32 selector;
	u8 type;
	u8 dpl;
	u8 present;

	present = ((*(u8*)(buffer + 5)) & 0x80) >> 7;
	dpl = ((*(u8*)(buffer + 5)) & 0x60) >> 5;
	type = ((*(u8*)(buffer + 5)) & 0x1F);

	offset = *(u16*)buffer;
	offset += (*(u16*)(buffer + 6)) << 16;

	selector = *(u16*)(buffer + 2);

	if (type == 0x0E)
	{
		// Interrupt Gate
		printk("0x%08x[index=%04X, TI=%d, RPL=%d] : 0x%08x, ", selector, selector >> 3, selector&0x0004, selector&0x0003, offset);
		printk("P[%d] ", present);
		printk("DPL[%d] ", dpl);
		printk("Type[%16s] ", "Interrupt Gate");
		printk("\n");
	}
	else if (type == 0x0F)
	{
		// Trap Gate
		printk("0x%08x[index=%04X, TI=%d, RPL=%d] : 0x%08x, ", selector, selector >> 3, selector&0x0004, selector&0x0003, offset);
		printk("P[%d] ", present);
		printk("DPL[%d] ", dpl);
		printk("Type[%16s] ", "Trap Gate");
		printk("\n");
	}
	else if (type == 0x05)
	{
		// Task Gate
		printk("0x%08x[index=%04X, TI=%d, RPL=%d] : 0x%08x, ", selector, selector >> 3, selector&0x0004, selector&0x0003, offset);
		printk("P[%d] ", present);
		printk("DPL[%d] ", dpl);
		printk("Type[%16s] ", "Task Gate");
		printk("\n");
	}
	else
	{
		printk("[null]\n");
	}
}

void analyzeIDT(u32 size, char* buffer)
{
	int i;
	for (i = 0; i < size; i += 8)
	{
		printk("[%4X] ", i/8);	
		analyzeIDTEntry(buffer + i);
	}
}

#define getNormalReg(reg, val) asm("movl %%"#reg",%0" : "=r" (val));

#define dumpNormalReg(reg) {u32 val;getNormalReg(reg, val); printk("%08s:0x%08X\n", ""#reg"", val);}

#define dumpLDT() {u32 val; asm("sldt %0" : "=r"(val)); printk("%08s:0x%08X\n", "ldt", val);}
#define dumpTSS() {u32 val; asm("str %0" : "=r"(val));  printk("%08s:0x%08X\n", "tss", val);}

#define dumpGDT() {char gdt[6]; \
	asm("sgdt %0" : "=m"(gdt)); \ 
	printRawData(6, gdt); \
	printk("%08s:0x%08X(0x%04X)\n", "gdt", *(u32*)(gdt + 2), *(u16*)(gdt)); \ 
	printRawData((*(u16*)(gdt) + 1), (u_char*)(*(u32*)(gdt + 2))); \ 
	analyzeGDT((*(u16*)(gdt) + 1), (u_char*)(*(u32*)(gdt + 2)));}

#define dumpIDT() {char idt[6]; \
	asm("sidt %0" : "=m"(idt)); \ 
	printRawData(6, idt); \
	printk("%08s:0x%08X(0x%04X)\n", "idt", *(u32*)(idt + 2), *(u16*)(idt)); \ 
	printRawData((*(u16*)(idt) + 1), (u_char*)(*(u32*)(idt + 2))); \ 
	analyzeIDT((*(u16*)(idt) + 1), (u_char*)(*(u32*)(idt + 2)));}

void analyzeRegisters()
{
	int cpu = smp_processor_id();
	struct desc_struct *gdt = get_cpu_gdt_table(cpu);
	printRawData(sizeof(*gdt), gdt);

	dumpNormalReg(cr0);
	dumpNormalReg(cr2);
	dumpNormalReg(cr3);

	dumpNormalReg(eax);
	dumpNormalReg(ebx);
	dumpNormalReg(ecx);
	dumpNormalReg(edx);

	dumpNormalReg(esp);
	dumpNormalReg(ebp);
	
	dumpNormalReg(esi);
	dumpNormalReg(edi);

	dumpTSS();
	dumpLDT();

	dumpGDT();
	dumpIDT();
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
		printk("[0x%08X] ", pageTableAddr);
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

	for (i = 0; i < size/4; ++i)
	{
		//printk("%08x ", i);
		printk("[0x%08x - 0x%08x] ", i << PGDIR_SHIFT, (((i+1) << PGDIR_SHIFT) - 1));
		dumpPageDirectoryEntry(*(entries + i));
	}
}

void analyzePhysicalPages()
{
	// printRawData(0x4000, mem_map);
	// printk("%08x\n", 896 * 1024 * 1024);
	// printk("%08x\n", high_memory);
	// struct page* first_page;
	// int i;

	// first_page = mem_map;
	
	// for (i = 0; i < 0x1000; ++i)
	// {
	// 	unsigned long bLocked = (first_page + i)->flags == PG_locked;
	// 	if (!bLocked)
	// 	{
	// 		printk("0x%08x[%08x]\n", page_address(first_page + i), (first_page + i)->flags);
	// 	}
	// 	first_page++;
	// }

	u32 val;
	getNormalReg(cr3, val);
	printk("CR3: 0x%08X\n", val);

	printk("high_memory: 0x%08X\n", high_memory);
	printk("PAGE_OFFSET: 0x%08X\n", PAGE_OFFSET);

	if (val < high_memory)
	{
		//printRawData(1 << PAGE_SHIFT, __va(val));
		dumpPageDirectory(1 << PAGE_SHIFT, __va(val));
	}
	
	// printk("Kernel Direct Mapped Area: [%d - %d]\n", min_low_pfn, max_low_pfn);
	//printRawData(0x100, val);
}

void analyzeTaskPgd()
{
	struct task_struct *pos;
    struct list_head *current_head;

	current_head=&(current->tasks);
	if (current_head == NULL)
		return 0;
	list_for_each_entry(pos,current_head,tasks)
	{
	    if (pos == NULL || pos->mm == NULL)
        	continue;
	    
        //printk("[%8d] : [0x%08X] %s\n",pos->pid, pos->mm->pgd, pos->comm);

        if (strcmp(pos->comm, "gnome-terminal") == 0)
        {
        	dumpPageDirectory(1 << PAGE_SHIFT, pos->mm->pgd);
        }
	}
}

union _cpuid4_leaf_eax {
	struct {
		unsigned int	type:5;
		unsigned int		level:3;
		unsigned int		is_self_initializing:1;
		unsigned int		is_fully_associative:1;
		unsigned int		reserved:4;
		unsigned int		num_threads_sharing:12;
		unsigned int		num_cores_on_die:6;
	} split;
	u32 full;
};

union _cpuid4_leaf_ebx {
	struct {
		unsigned int		coherency_line_size:12;
		unsigned int		physical_line_partition:10;
		unsigned int		ways_of_associativity:10;
	} split;
	u32 full;
};

union _cpuid4_leaf_ecx {
	struct {
		unsigned int		number_of_sets:32;
	} split;
	u32 full;
};

void cpuidTest()
{
	u32 msr_edx, msr_eax;
	
	union _cpuid4_leaf_eax val_eax;
	union _cpuid4_leaf_ebx val_ebx;
	union _cpuid4_leaf_ecx val_ecx;
	u32 val_edx; 
	asm("cpuid"
		    : "=a" (val_eax),
		      "=b" (val_ebx),
		      "=c" (val_ecx),
		      "=d" (val_edx)
		    : "a" (4), "c"(2));

	u32 ways,partitions,line_Size, sets;

	ways = val_ebx.split.ways_of_associativity;
	partitions = val_ebx.split.physical_line_partition;
	line_Size = (val_ebx).split.coherency_line_size;
	sets = val_ecx.split.number_of_sets;

	printk("eax: 0x%08X\n", val_eax);
	printk("ebx: 0x%08X\n", val_ebx);
	printk("ecx: 0x%08X\n", val_ecx);
	printk("edx: 0x%08X\n", val_edx);

	printk("ways: %d\n", ways+1);
	printk("partitions: %d\n", partitions+1);
	printk("line_size: %d\n", line_Size+1);
	printk("sets: %d\n", sets+1);
	printk("Cache L3 size: %d KB\n", ((ways + 1)*(partitions + 1)*(line_Size + 1)*(sets + 1)) / 1024);

	
	// asm("rdmsr"
	// 	    : "=a" (msr_eax),
	// 	      "=d" (msr_edx)
	// 	    : "c"(0x174));

	// printk("edx:eax = %08x:%08x\n", msr_edx, msr_eax);

	// asm("rdmsr"
	// 	    : "=a" (msr_eax),
	// 	      "=d" (msr_edx)
	// 	    : "c"(0x175));

	// printk("edx:eax = %08x:%08x\n", msr_edx, msr_eax);

	// asm("rdmsr"
	// 	    : "=a" (msr_eax),
	// 	      "=d" (msr_edx)
	// 	    : "c"(0x176));

	// printk("edx:eax = %08x:%08x\n", msr_edx, msr_eax);

	u64 sysenter_cs = native_read_msr(0x174);
	printk("%016d\n", sysenter_cs);

	u64 sysenter_esp = native_read_msr(0x175);
	printk("%016d\n", sysenter_esp);

	u64 sysenter_eip = native_read_msr(0x176);
	printk("%016d\n", sysenter_eip);

	u64 timestamp = native_read_msr(0x10);
	printk("%016d\n", timestamp);


}

void netanalyze()
{
	struct softnet_data *sd = &per_cpu(softnet_data, 0);
	printRawData(sizeof(*sd), sd);

	struct sk_buff_head*	input_pkt_queue = &sd->input_pkt_queue;
	printk("Total %d packets in incoming queue\n", input_pkt_queue->qlen);

	printk("processed %d \n", sd->processed);
	printk("received %d \n", sd->received_rps);

	struct sk_buff_head*	complete_queue = &sd->completion_queue;
	printk("Total %d packets in complete queue\n", complete_queue->qlen);
}

void analyzeTCPHeader(struct tcphdr* tcp_hdr)
{
	if (tcp_hdr == NULL)
		return;
	
	printk("**********************TCP Header***********\n");
	printk("%30s:\t0x%04x\n", "Source", 
			tcp_hdr->source);

	printk("%30s:\t0x%04x\n", "Destination", 
			tcp_hdr->dest);

	printk("%30s:\t0x%04x\n", "seq", 
			tcp_hdr->seq);

	printk("%30s:\t0x%04x\n", "ack_seq", 
			tcp_hdr->ack_seq);

	printk("%30s:\t0x%04x\n", "Window", 
			tcp_hdr->window);

	printk("%30s:\t0x%04x\n", "Checksum", 
			tcp_hdr->check);

	printk("%30s:\t0x%04x\n", "urg_ptr", 
			tcp_hdr->urg_ptr);
	
}

void analyzeIPHeader(struct iphdr* ip_hdr)
{
	printk("**********************IP Header***********\n");
	printk("%30s:\t0x%02x\n", "Version", 
			ip_hdr->version);

	printk("%30s:\t0x%02x (%u)\n", "Header Length(Bytes)", 
		ip_hdr->ihl, 
		ip_hdr->ihl);
	
	printk("%30s:\t0x%02x\n", "Type of service", 
		ip_hdr->tos);

	printk("%30s:\t0x%04x (%u)\n", "Total Length(Bytes)", 
		ip_hdr->tot_len, 
		ip_hdr->tot_len);

	printk("%30s:\t0x%04x (%u)\n", "Identification", 
		ip_hdr->id,
		ip_hdr->id);

	printk("%30s:\t0x%04x (%u)\n", "Fragment Offset", 
		ip_hdr->frag_off,
		ip_hdr->frag_off);

	printk("%30s:\t0x%02x\n", "Time to live", 
		ip_hdr->ttl);

	printk("%30s:\t0x%02x", "Protocol", 
		ip_hdr->protocol);

	
	if (ip_hdr->protocol == 0x11)
	{
		printk(" [UDP]\n");
	}
	else if (ip_hdr->protocol == 0x06)
	{
		printk(" [TCP]\n");
	}
	else if (ip_hdr->protocol == 0x01)
	{
		printk(" [ICMP]\n");
	}
	else if (ip_hdr->protocol == 0x02)
	{
		printk(" [IGMP]\n");
	}

	printk("%30s:\t0x%04x\n", "Header Checksum (CRC)", 
		ip_hdr->check);

	printk("%30s:\t%u:%u:%u:%u\n", "Source IP Address", 
		*(unsigned char*)(((unsigned char*)&ip_hdr->saddr) + 0),
		*(unsigned char*)(((unsigned char*)&ip_hdr->saddr) + 1),
		*(unsigned char*)(((unsigned char*)&ip_hdr->saddr) + 2),
		*(unsigned char*)(((unsigned char*)&ip_hdr->saddr) + 3)
		);

	printk("%30s:\t%u:%u:%u:%u\n", "Destination IP Address", 
		*(unsigned char*)(((unsigned char*)&ip_hdr->daddr) + 0),
		*(unsigned char*)(((unsigned char*)&ip_hdr->daddr) + 1),
		*(unsigned char*)(((unsigned char*)&ip_hdr->daddr) + 2),
		*(unsigned char*)(((unsigned char*)&ip_hdr->daddr) + 3)
		);

}

void analyzePacket(struct sk_buff* sk_buffer)
{
	sk_buff_data_t		transport_header;
	sk_buff_data_t		network_header;
	sk_buff_data_t		mac_header;
	sk_buff_data_t		tail;
	sk_buff_data_t		end;
	unsigned char		*head, *data;
	unsigned int		truesize;

	transport_header = sk_buffer->transport_header;
	network_header = sk_buffer->network_header;
	mac_header = sk_buffer->mac_header;

	end = sk_buffer->end;
	tail = sk_buffer->tail;

	head = sk_buffer->head;
	data = sk_buffer->data;

	truesize = sk_buffer->truesize;

	printk("**********************struct sk_buff***********\n");

	printk("%16s : 0x%08X\n", "transport_header", transport_header);
	printk("%16s : 0x%08X\n", "network_header", network_header);
	printk("%16s : 0x%08X\n", "mac_header", mac_header);

	printk("%16s : 0x%08X\n", "end", end);
	printk("%16s : 0x%08X\n", "tail", tail);

	printk("%16s : 0x%08X\n", "head", head);
	printk("%16s : 0x%08X\n", "data", data);

	printk("%16s : 0x%08X\n", "truesize", truesize);

	printRawData(tail - data, data);

}

void findBuffHead(struct sk_buff *skb)
{
	struct sk_buff_head* head;
	struct sk_buff* buf;
	unsigned int packets = 0;

	if (skb->next != NULL)
	{
		printk("next: 0x%08x\n", skb->next);
	}
	//printk("next: 0x%08x\n", skb->next);

	// buf = skb;
	// do
	// {
	// 	packets++;

	// 	buf = buf->next;
	// }
	// while (buf != skb && buf->next != NULL);

	// head = (struct sk_buff_head*)buf;
	// printk("We found the head, total %d nodes in this queue\n", packets);
}

unsigned int hook_func(unsigned int hooknum,
	struct sk_buff *skb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph ;
	

	if (skb != NULL)
	{
		//findBuffHead(skb);
		iph = ip_hdr(skb);

		// // printk("sk_buff : 0x%08x ip_hdr : 0x%08x \n", sb, iph);
		if (iph != NULL)
		{
			// printk("ip: %d:%d\n", iph->saddr, iph->daddr);
			analyzeIPHeader(iph);			

			if (iph->protocol == 0x06)
			{
				analyzeTCPHeader(tcp_hdr(skb));
			}
		}	
		analyzePacket(skb);
	}

	return NF_ACCEPT;	
}


static struct nf_hook_ops hook_ops = {
	.hook = hook_func,
	.hooknum = NF_INET_PRE_ROUTING,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};

void AnalyzeSuperBlock()
{
	struct super_block* item;

	list_for_each_entry(item,&super_blocks,s_list)
	{
		printk("super_block 0x%08X\n", item);
	}
}

static int pslist_init()
{
	printk("###################################################################\n");

	// analyzeRegisters();
	// analyzeUMANode();
	// analyzeProcesses();
	// analyzePhysicalPages();
	// analyzeTaskPgd();
	// cpuidTest();
	// netanalyze();
	// nf_register_hook(&hook_ops);
	AnalyzeSuperBlock();
	return 0;
}

static void pslist_exit()
{
	nf_unregister_hook(&hook_ops);
	printk("###################################################################\n");
}



module_init(pslist_init);
module_exit(pslist_exit);
