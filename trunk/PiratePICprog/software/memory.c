#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#include "memory.h"

static struct mem_page_t *MEM_CreatePage(struct memory_t *mem, uint32_t base)
{
	struct mem_page_t *tmp = NULL;

	tmp = safe_malloc(sizeof(struct mem_page_t));

	tmp->base = base;
	tmp->size = 0;
	tmp->next = NULL;
	//tmp->data = NULL;

	tmp->data = safe_malloc(mem->page_size);
	memset(tmp->data, MEM_EMPTY, mem->page_size);

	return tmp;
}

static void MEM_PageTrim(struct mem_page_t *page)
{
	int32_t i;

	if (page == NULL)
		return;

	if (page->data == NULL)
		return;

	i = page->size - 1;
	while (i >= 0 && page->data[i] == MEM_EMPTY) {
		i--;
	}

	page->size = i + 1;

}

struct memory_t *MEM_Init(uint32_t page_size)
{
	struct memory_t *tmp;

	tmp = safe_malloc(sizeof(struct memory_t));

	tmp->page = NULL;
	tmp->page_size = page_size;

	return tmp;
}

int MEM_PageExists(struct memory_t *mem, uint32_t addr)
{
	struct mem_page_t *next = NULL;
	struct mem_page_t *cur = mem->page;
	uint32_t base;

	base = addr & ~(mem->page_size - 1);

	if (cur == NULL) {
		return 0;
	}

	while ((cur->next != NULL) && (cur->next->base <= base)) {
		cur = cur->next;
	}

	if (cur->base == base) {
		return 1;
	}

	return 0;
}

struct mem_page_t *MEM_GetPage(struct memory_t *mem, uint32_t base)
{
	struct mem_page_t *next = NULL;
	struct mem_page_t *cur = mem->page;

	if (base & (mem->page_size-1)) {
		return NULL;
	}

	if (cur == NULL) {
		cur = MEM_CreatePage(mem, base);
		mem->page = cur;
		return cur;
	}

	while ((cur->next != NULL) && (cur->next->base <= base)) {
		cur = cur->next;
	}

	if (cur->base == base) {
		return cur;
	}

	next = MEM_CreatePage(mem, base);

	if (cur->next != NULL) {
		next->next = cur->next;
	}
	cur->next = next;

	return next;
}

struct mem_page_t *MEM_GetFirstPage(struct memory_t *mem)
{
	if (mem == NULL)
		return NULL;

	return mem->page;
}

struct mem_page_t *MEM_GetNextPage(struct mem_page_t *page)
{
	if (page == NULL)
		return NULL;
	
	return page->next;
}

int MEM_Write(struct memory_t *mem, uint32_t addr, uint8_t *data, uint32_t len)
{
	struct mem_page_t *page;
	uint32_t base;
	uint32_t offset;

	base = addr & ~(mem->page_size - 1);

	page = MEM_GetPage(mem, base);

	if (page->base + mem->page_size < addr + len) {
		uint32_t rest;
		rest = (addr + len) - (page->base + mem->page_size);
		len = len - rest;

		MEM_Write(mem, addr + len, data + len, rest);
	}

#ifdef DEBUG
	printf("@%08x %08x %02x (%p) = %02x\n", addr, page->base, page->size, (void *)page, data[0]);
#endif

	offset = addr - base;
	memcpy(page->data + offset, data, len);

	if (page->size < offset + len) {
		page->size = offset + len;
	}

	return 0;
}

int MEM_Read(struct memory_t *mem, uint32_t addr, uint8_t **data, uint32_t len)
{
	struct mem_page_t *page;
	uint32_t base;
	uint32_t offset;

	base = addr & ~(mem->page_size - 1);
	offset = addr - base;

	if (offset + len > mem->page_size) {
		return -1;
	}

	if (MEM_PageExists(mem, base) == 0) {
		return -2;
	}

	page = MEM_GetPage(mem, base);

	if (offset + len > page->size) {
		return 0;
	}

	*data = &(page->data[offset]);

	return len;
}

// TODO: test
int MEM_Compare(struct memory_t *mem_a, struct memory_t *mem_b)
{
	struct mem_page_t *pa;
	struct mem_page_t *pb;

	pa = mem_a->page;
	pb = mem_b->page;

	if (mem_a->page_size != mem_b->page_size) {
		return 1;
	}

	while ((pa != NULL) && (pb != NULL)) {
		MEM_PageTrim(pa);
		MEM_PageTrim(pb);

		if (pa->size != pb->size) {
			printf("page data size differs page=0x%08x\n",pa->base);
			return 1;
		}

		if (pa->base != pb->base) {
			printf("page base differs\n");
			return 1;
		}

		if (memcmp(pa->data, pb->data, pa->size) != 0) {
			printf("page data differs\n");
			return 1;
		}

		/* fastforward to non-empty pages */
		do {
			pa = MEM_GetNextPage(pa);
		} while ((pa != NULL) && MEM_PageEmpty(pa));

		do {
			pb = MEM_GetNextPage(pb);
		} while ((pb != NULL) && MEM_PageEmpty(pb));
	}

	if ((pa != NULL) || (pb != NULL)) {
#ifdef DEBUG
		if (pb != NULL) {
			printf(" page b @0x%08x\n", pb->base);
		}
		if (pa != NULL) {
			printf(" page a @0x%08x\n", pa->base);
		}
#endif
		return 1;
	}

	return 0;

}

int MEM_PageEmpty(struct mem_page_t *page)
{
	uint32_t i;

	for (i = 0; i < page->size; i++) {
		if (page->data[i] != MEM_EMPTY) {
			return 0;
		}
	}

	return 1;
}

void MEM_Optimize(struct memory_t *mem)
{
	struct mem_page_t *cur = mem->page;

	while (cur != NULL) {
		MEM_PageTrim(cur);
		cur = cur->next;
	}

}

void MEM_Print(struct memory_t *mem)
{
	struct mem_page_t *cur = mem->page;

	uint32_t size = 0;

	while (cur != NULL) {
		MEM_PageTrim(cur);
		if (cur->size > 0) {
			printf("\npage @0x%08x (size 0x%04x)\n", cur->base, cur->size);
			size += cur->size;

			if (cur->data !=NULL) {
				int i;
				//for (i = 0; i < mem->page_size; i++) {
				for (i = 0; i < cur->size; i++) {
					printf("%02x ", cur->data[i]);
					if (((i % 16) == 15)) {
						printf("\n");
					}
				}
				printf("\n");
			}
		}
		cur = cur->next;
	}

	printf("Total size: %ld \n", (long int)size);
}

int MEM_Destroy(struct memory_t *mem)
{
	struct mem_page_t *next = NULL;
	struct mem_page_t *cur = mem->page;

	while (cur != NULL) {
		next = cur->next;
		cur->next = NULL;

		if (cur->data !=NULL) {
			free(cur->data);
			cur->data = NULL;
		}

		free(cur);
		cur = next;
	}

	mem->page = NULL;
	free(mem);

	return 0;
}

