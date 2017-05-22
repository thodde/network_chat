#include "common.h"

int compare(thread_info_struct *a, thread_info_struct *b);
void list_init(linked_list *ll);
int list_insert(linked_list *ll, thread_info_struct *thr_info);
int list_delete(linked_list *ll, thread_info_struct *thr_info);
void list_dump(linked_list *ll);