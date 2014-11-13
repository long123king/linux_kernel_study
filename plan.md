# Plan #
1. get current process struct **task_struct**.
2. get all files struct **file** via `task_struct->files->fd_array`.
3. get struct **address_space**.
4. get all pages that is present in `address_space->page_tree`.  
   `page = radix_tree_lookup(&mapping->page_tree, page_offset);` is use case.   
   we can pass page_offset as 0, 1, ..... series.
   and we got struct **page**.
5. how to get page content via struct **page**?
   all the struct **page** instances are stored in **mem_map** array, 
   and the array will consume `32/4096` portion of all the physical memory.
   `#define __page_to_pfn(page)    ((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET)`
   is the macro to get **pfn**(Page Frame Number) from struct **page**.

> **pfn** is physical Page Frame Number, so we cannot use it to read content of the page.
> **virtual pfn** is translated to *physical pfn* by CPU with the help of **pgdir/pgtable**.
> **inode** is connected with per-process virtual memory layout with **vm_area_struct**.
> an **inode** can be mapped from several **vm_area_struct**, each has a different virtual address.
> so maybe we got the wrong way.

****************************************************************************
    /*
     * Remove any clean buffers from the inode's buffer list.  This is called
     * when we're trying to free the inode itself.  Those buffers can pin it.
     *
     * Returns true if all buffers were removed.
     */
    int remove_inode_buffers(struct inode *inode)
    {
    	int ret = 1;
    
    	if (inode_has_buffers(inode)) {
    		struct address_space *mapping = &inode->i_data;
    		struct list_head *list = &mapping->private_list;
    		struct address_space *buffer_mapping = mapping->assoc_mapping;
    
    		spin_lock(&buffer_mapping->private_lock);
    		while (!list_empty(list)) {
    			struct buffer_head *bh = BH_ENTRY(list->next);
    			if (buffer_dirty(bh)) {
    				ret = 0;
    				break;
    			}
    			__remove_assoc_queue(bh);
    		}
    		spin_unlock(&buffer_mapping->private_lock);
    	}
    	return ret;
    }   
****************************************************************************

so `address_space->private_list` is pointing to a list of **buffer_head**.
