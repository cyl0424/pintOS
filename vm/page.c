#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "threads/interrupt.h"
#include <string.h>

void vm_init (struct hash *vm){
    if (vm != NULL){
        hash_init(vm, vm_hash_func, vm_less_func, NULL);  
    }
    return ;
}

static unsigned vm_hash_func (const struct hash_elem *e, void *aux UNUSED){
    int res;
    if (e != NULL){
        int hash_ent = hash_entry(e, struct vm_entry, elem) ->vaddr;
        res = hash_int(hash_ent);
        return res;
    }
    return ;
}

static bool vm_less_func (const struct hash_elem *e1, const struct hash_elem *e2, void *aux UNUSED){
    if (e1 != NULL && e2 != NULL){
        int e1_entry = hash_entry(e1, struct vm_entry, elem) ->vaddr;
        int e2_entry = hash_entry(e2, struct vm_entry, elem) ->vaddr;
        return e1_entry < e2_entry;
    }
    return ;
}

static void vm_destroy_func (struct hash_elem *e, void *aux UNUSED){
    ASSERT (e != NULL);
    struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
    free_page(pagedir_get_page(thread_current()->pagedir, vme->vaddr));
    free(vme);
}

void vm_destroy (struct hash *vm){
    if(vm != NULL){
        hash_destroy(vm, vm_destroy_func);
    }
}

struct vm_entry *find_vme (void *vaddr){
    struct vm_entry vme;
    struct hash_elem *e;
    struct thread *cur = thread_current();

    if (vaddr != NULL){
        vme.vaddr = pg_round_down(vaddr);

        if (pg_ofs(vme.vaddr) == 0){
            e = hash_find(&cur->vm, &vme.elem);
        }
    }

    if (e != NULL){
        struct vm_entry *res = hash_entry(e, struct vm_entry, elem);
        return res;
    }
    else {
        return NULL;
    }
}


bool insert_vme (struct hash *vm, struct vm_entry *vm_entry){
    if(vm != NULL && vm_entry != NULL && pg_ofs(vm_entry->vaddr)==0){
        vm_entry->pinned_flag = false;
        bool res = (hash_insert(vm, &vm_entry->elem) == NULL);
        return res;
    }
    return ;
}

bool delete_vme (struct hash *vm, struct vm_entry *vm_entry){
    if(vm != NULL && vm_entry != NULL){
        if(!hash_delete(vm, &vm_entry->elem)){
            return false;
        };

        return true;
    }
    return ;
}

bool load_file (void *kaddr, struct vm_entry *vm_entry){
    if (kaddr == NULL || vm_entry == NULL || vm_entry -> type == VM_ANON){
        return;
    }
    if (file_read_at(vm_entry->file, kaddr, vm_entry->read_bytes, vm_entry->offset) != (int) vm_entry->read_bytes){
        return false;
    }

    memset(kaddr + (vm_entry->read_bytes), 0, vm_entry->zero_bytes);
    return true;
}

void free_victim_page(enum palloc_flags flag){
    struct page *page;
    struct page *victim;

    if(list_empty(&lru_list)){
        lru_clock = NULL;
    }
    else if(lru_clock==NULL || lru_clock==list_end(&lru_list)){
        lru_clock = list_begin(&lru_list);
    }else{
        lru_clock = list_next(lru_clock);
    }

    page = list_entry(lru_clock, struct page, lru);

    while(page->vme->pinned_flag || pagedir_is_accessed(page->t->pagedir, page->vme->vaddr)){
        pagedir_is_accessed(page->t->pagedir, page->vme->vaddr);
        if(list_empty(&lru_list)){
        lru_clock = NULL;
        }
        else if(lru_clock==NULL || lru_clock==list_end(&lru_list)){
            lru_clock = list_begin(&lru_list);
        }else{
            lru_clock = list_next(lru_clock);
        }

        page = list_entry(lru_clock, struct page, lru);
    }

    victim = page;
    uint32_t victim_pgd = victim->t->pagedir;
    struct vm_entry *victim_vaddr = victim->vme->vaddr;

    switch (victim
    ->vme->type)
    {
    case VM_BIN:
        if(pagedir_is_dirty(victim_pgd, victim_vaddr)){
            victim->vme->swap_slot = swap_out(victim->kaddr);
            victim->vme->type = VM_ANON;
        }
        break;
    case VM_FILE:
        if(pagedir_is_dirty(victim_pgd, victim_vaddr)){
            file_write_at(victim->vme->file, victim->vme->vaddr, victim->vme->read_bytes, victim->vme->offset);
        }
        break;
    case VM_ANON:
        victim->vme->swap_slot = swap_out(victim->kaddr);
        break;
    default:
        break;
    }
    victim->vme->is_loaded = false;
    __free_page(victim);
}

struct page *alloc_page (enum palloc_flags flag){
    lock_acquire(&lru_list_lock);
    int *kpage;
    kpage = palloc_get_page(flag);
    while (kpage == NULL){
        free_victim_page(flag);
        kpage = palloc_get_page(flag);
    }

    struct page *page;
    page = (struct page *)malloc(sizeof(struct page));
    page->kaddr = kpage;
    page->t = thread_current();

    add_page_to_lru_list(page);
    lock_release(&lru_list_lock);

    return page;
}

void free_page (void *kaddr){
    struct list_elem *e;
    struct page *page;
    bool find = false;

    lock_acquire(&lru_list_lock);
    for(e = list_begin(&lru_list); e != list_end(&lru_list); e = list_next(e)){
        page = list_entry(e, struct page, lru);
        if(page->kaddr == kaddr){
            find = true;
            break;
        }
    }
    if (find == true && page!= NULL){
        __free_page(page);
    }

    lock_release(&lru_list_lock);
}

void __free_page (struct page *p){
    del_page_from_lru_list(p);
    pagedir_clear_page(p->t->pagedir, pg_round_down(p->vme->vaddr));
    palloc_free_page(p->kaddr);
    free(p);
}