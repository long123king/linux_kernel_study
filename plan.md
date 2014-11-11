# Plan #
1. get current process struct **task_struct**.
2. get all files struct **file** via `task_struct->files->fd_array`.
3. get struct **address_space**.
4. get all pages that is present in `address_space->page_tree`.  
   `page = radix_tree_lookup(&mapping->page_tree, page_offset);` is use case.  
   we can pass page_offset as 0, 1, ..... series.
   and we got struct **page**.
5. how to get page content via struct **page**?
