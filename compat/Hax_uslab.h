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
 */

#ifndef _HAX_USLAB_H_
#define _HAX_USLAB_H_

#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

#ifndef UINT64_T_DEFINED
typedef __UINT64_TYPE__ Uint64_t;
#define UINT64_T_DEFINED
#endif

struct Hax_uslab_pt {
	/*
	 * first_free and generation *must* be contiguous so that CAS2 can
	 * update both to avoid ABA conflicts on concurrent allocations.
	 */
	char	*first_free;
	char	*generation;
	Size_t	size;
	Size_t	used;
	Size_t	offset;

	char	 *base;
	/*
	 * Keep this cacheline-sized, otherwise false sharing will kill
	 * throughput in threads in adjacent uslabs.
	 */
	char	pad[64 - 48];
};

struct Hax_uslab_entry {
	char *next_free;
};

struct Hax_uslab {
	struct Hax_uslab_pt	*pt_base;
	char		*slab0_base;

	Uint64_t	size_class;
	Size_t		slab_len;
	Uint64_t	pt_slabs;
	Size_t		pt_size;
	Uint64_t	pt_ctr;
};

struct Hax_uslab 	*Hax_uslab_create_heap(Size_t size_class, Uint64_t nelem, Uint64_t npt_slabs);

void		*Hax_uslab_alloc(struct Hax_uslab *);
void		Hax_uslab_free(struct Hax_uslab *, void *p);

void		Hax_uslab_destroy_heap(struct Hax_uslab *);

#endif
