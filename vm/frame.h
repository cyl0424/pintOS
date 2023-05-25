#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm/page.h"
#include <hash.h>
#include <list.h>
#include "threads/synch.h"
#include "threads/palloc.h"

struct list lru_list;
struct lock lru_list_lock;
struct list_elem *lru_clock;

void lru_list_init (void);
void add_page_to_lru_list (struct page *);
void del_page_from_lru_list (struct page *);

#endif