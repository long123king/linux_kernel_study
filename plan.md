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
   
