#include "vm/frame.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/interrupt.h"
#include "vm/page.h"
#include "threads/vaddr.h"


void lru_list_init(void){
    list_init(&lru_list);
    lock_init(&lru_list_lock);
    lru_clock = NULL;
}

void add_page_to_lru_list (struct page *page){
    lock_acquire(&lru_list_lock);
    list_push_back(&lru_list, &page->lru);
    lock_release(&lru_list_lock);
}

void del_page_from_lru_list (struct page *page){
    if(&page->lru == lru_clock){
        lru_clock = list_next(lru_clock);
    }
    list_remove(&page->lru);
}

