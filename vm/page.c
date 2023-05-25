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
    free(hash_entry (e, struct vm_entry, elem));
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
        return hash_insert(vm, &vm_entry->elem) == NULL;
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

// struct page *alloc_page (enum palloc_flags);
// void free_page (void *);
// void free_page_thread (struct thread *t);
// void __free_page (struct page *p);