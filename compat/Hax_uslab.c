/*
 * Copyright 2015 Fastly, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Concurrent, in-line slab allocator implementation, safe for workloads with
 * a single concurrent process freeing and multiple concurrent processes
 * allocating.
 */

#include "Hax_uslab.h"

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef UINT64_T_DEFINED
typedef __UINT64_TYPE__ Uint64_t;
#define UINT64_T_DEFINED
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define PAGE_SIZE 4096

struct Hax_uslab_pt *Hax_uslab_pt;

void *Hax_CallocArena(Size_t number, Size_t size);

struct Hax_uslab *
Hax_uslab_create_heap(Size_t size_class, Uint64_t nelem, Uint64_t npt_slabs)
{
	char *cur_slab, *cur_base;
	struct Hax_uslab *a;
	Uint64_t i;

	if (((size_class * nelem) / npt_slabs) == 0) {
		return NULL;
	}

	a = Hax_CallocArena(1, (2 * PAGE_SIZE) + (size_class * nelem));
	if (a == NULL) {
		return NULL;
	}

	cur_slab = ((char *)a) + PAGE_SIZE;
	a->slab0_base = cur_base = ((char *)a) + (2 * PAGE_SIZE);

	a->pt_base = (struct Hax_uslab_pt *)cur_slab;
	a->pt_size = (size_class * nelem) / npt_slabs;
	a->pt_slabs = npt_slabs;
	a->size_class = size_class;
	a->slab_len = size_class * nelem;

	for (i = 0; i < npt_slabs; i++) {
		struct Hax_uslab_pt *pt;

		pt = (struct Hax_uslab_pt *)cur_slab;
		pt->base = pt->first_free = cur_base;
		pt->size = a->pt_size;
		pt->offset = i;

		cur_slab += sizeof (*pt);
		cur_base += a->pt_size;
	}

	return a;
}

void
Hax_uslab_destroy_heap(struct Hax_uslab *a)
{

//	free(a);
}

/*
 * When we begin, our slab is sparse and zeroed. Effectively, this means that
 * we obtain our memory either with mmap(2) and MAP_ANONYMOUS, by using
 * shm_open(3), ftruncate(2), and mmap(2), or the mmap(2)-backed file comes
 * from a RAM-backed storage that initializes to 0 on access.
 *
 * Our approach is to find the first free block. We then figure out what the
 * next free block will be. If the next free block is NULL, we know that the
 * block immediately following the block we've chosen is the next logically
 * free block.
 *
 * We are prone to ABA. If we read first_free, load the next_free from it, and
 * are subsequently pre-empted, another concurrent process could allocate and
 * then free our target. Additional allocations may have occurred which alter
 * the target's next_free member by the time it was freed. In this case, we
 * would end up in an inconsistent state. We solve this problem by doing a
 * CAS2 on our slab to update both the free block and a generation counter.
 */
void *
Hax_uslab_alloc(struct Hax_uslab *a)
{
	struct Hax_uslab_pt update, original, *slab, *oa;
	struct Hax_uslab_entry *target;
	char *next_free;

	if (Hax_uslab_pt == NULL) {
		Hax_uslab_pt = &a->pt_base[a->pt_ctr++ % a->pt_slabs];
	}

	slab = Hax_uslab_pt;

retry:
	/* If we're out of space, try to steal some memory from elsewhere */
	if (slab->first_free >= slab->base + slab->size) {
		Uint64_t i = 1;

		oa = slab;
		slab = &a->pt_base[(oa->offset + i) % a->pt_slabs];
		while (slab != oa && slab->first_free >= slab->base + slab->size) {
			slab = &a->pt_base[(oa->offset + i++) % a->pt_slabs];
		}

		/* OOM. */
		if (slab == oa) {
			return NULL;
		}

		goto retry;
	}

	original.generation = slab->generation;

	original.first_free = slab->first_free;
	target = (struct Hax_uslab_entry *)original.first_free;


	if (target->next_free == 0) {
		/*
		 * When this is the last block, this will put an address
		 * outside the bounds of the slab into the first_free member.
		 * If we succeed, no other threads could win the bad value as
		 * first_free is ABA protected and checked to be within bounds.
		 */
		next_free = original.first_free + a->size_class;
	} else {
		next_free = target->next_free;
	}

	update.generation = original.generation + 1;
	update.first_free = next_free;

	/*
	 * We failed to get the optimistic allocation, and our new
	 * first_free block is outside the bounds of this slab.
	 * Revert to trying to steal one from elsewhere.
	 */
	if (slab->first_free >= slab->base + slab->size) {
		slab = &a->pt_base[(slab->offset + 1) % a->pt_slabs];
		goto retry;
	}

	update.generation = original.generation + 1;
	target = (struct Hax_uslab_entry *)original.first_free;

	if (target->next_free == 0) {
		next_free = original.first_free + a->size_class;
	} else {
		next_free = target->next_free;
	}

	update.first_free = next_free;

	return target;
}

/*
 * An slab free routine that is safe with one or more concurrent unique
 * freeing processes in the face of many concurrent allocating processes. We
 * don't need any CAS2 voodoo here because we do not rely on the value of
 * next_free for the entry we are attempting to replace at the head of our
 * stack. Additionally, it is impossible for us to observe the same value
 * at the time we read target and the time we try to write to it because
 * no other concurrent processes know about target.
 */
void
Hax_uslab_free(struct Hax_uslab *a, void *p)
{
	struct Hax_uslab_pt *allocated_slab;
	struct Hax_uslab_entry *e;
	char *target;

	/* Stupid. */
	if (p == NULL) return;

	/*
	 * We want to free these into the same section of the pool from which
	 * they were allocated.
	 */
	allocated_slab = &a->pt_base[(((char *)p) - a->slab0_base) / a->pt_size];

	e = p;
	target = allocated_slab->first_free;
	e->next_free = target;
}
