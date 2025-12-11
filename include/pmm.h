#include "types.h"
#pragma once

void pmm_init();
void *alloc_page();
void free_page(void *pa);

void test_pmm();
