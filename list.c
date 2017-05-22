#include "list.h"
 
int compare(thread_info_struct *a, thread_info_struct *b) {
    return a->sockfd - b->sockfd;
}
 
void list_init(linked_list *ll) {
    ll->head = ll->tail = NULL;
    ll->size = 0;
}
 
int list_insert(linked_list *ll, thread_info_struct *thr_info) {
    if(ll->size == CLIENTS) return -1;
    if(ll->head == NULL) {
        ll->head = (node *)malloc(sizeof(node));
        ll->head->threadinfo = *thr_info;
        ll->head->next = NULL;
        ll->tail = ll->head;
    }
    else {
        ll->tail->next = (node*)malloc(sizeof(node));
        ll->tail->next->threadinfo = *thr_info;
        ll->tail->next->next = NULL;
        ll->tail = ll->tail->next;
    }
    ll->size++;
    return 0;
}
 
int list_delete(linked_list *ll, thread_info_struct *thr_info) {
    node *curr, *temp;
    if(ll->head == NULL) return -1;
    if(compare(thr_info, &ll->head->threadinfo) == 0) {
        temp = ll->head;
        ll->head = ll->head->next;
        if(ll->head == NULL) ll->tail = ll->head;
        free(temp);
        ll->size--;
        return 0;
    }
    for(curr = ll->head; curr->next != NULL; curr = curr->next) {
        if(compare(thr_info, &curr->next->threadinfo) == 0) {
            temp = curr->next;
            if(temp == ll->tail) ll->tail = curr;
            curr->next = curr->next->next;
            free(temp);
            ll->size--;
            return 0;
        }
    }
    return -1;
}
 
void list_dump(linked_list *ll) {
    node *curr;
    thread_info_struct *thr_info;
	// show the current list size
    printf("Connection count: %d\n", ll->size);
	
	// iterate over the list and display all the connected users
    for(curr = ll->head; curr != NULL; curr = curr->next) {
        thr_info = &curr->threadinfo;
        printf("[%d] %s\n", thr_info->sockfd, thr_info->alias);
    }
}