#include "vm/swap.h"
#include <bitmap.h>
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "devices/block.h"
#include "threads/vaddr.h"
#include "vm/frame.h"
#include "vm/page.h"

struct lock swap_lock;
struct bitmap *swap_bitmap;

void swap_init(size_t size){
    lock_init(&swap_lock);
    swap_bitmap = bitmap_create(size);
}

void swap_in(size_t index, void *kaddr){
    lock_acquire(&swap_lock);
    struct block *swap_block = block_get_role(BLOCK_SWAP);
    if(bitmap_test(swap_bitmap, index)){
        int i;
        for(i=0; i<PGSIZE/BLOCK_SECTOR_SIZE; i++){
            block_read(swap_block, PGSIZE/BLOCK_SECTOR_SIZE*index + i, BLOCK_SECTOR_SIZE*i+kaddr);
        }
        bitmap_reset(swap_bitmap, index);
    }
    lock_release(&swap_lock);
}


size_t swap_out(void *kaddr){
    lock_acquire(&swap_lock);
    struct block *swap_block = block_get_role(BLOCK_SWAP);

    size_t swap_index = bitmap_scan(swap_bitmap, 0, 1, false);

    if(BITMAP_ERROR != swap_index){
        int i;
        for(i=0; i<PGSIZE/BLOCK_SECTOR_SIZE; i++){
            block_write(swap_block, PGSIZE/BLOCK_SECTOR_SIZE*swap_index + i, BLOCK_SECTOR_SIZE*i+kaddr);
        }
        bitmap_set(swap_bitmap, swap_index, true);
    }
    lock_release(&swap_lock);
    return swap_index;
}