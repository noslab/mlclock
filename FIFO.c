#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"BTREE.h"

int evict_cnt = 0;

struct node* find(int* cnt, long id, int idx, int mode, int* read_cnt){
    struct node* srch = search(id);
    if(srch == NULL){
        struct node* new = (struct node*)malloc(sizeof(struct node));
        new->id = id;
        new->prev = NULL;
        new->next = NULL;
        new->hit = 0;
        //new->first = idx;
        new->mode = mode;
        *cnt += 1;
        *read_cnt += 1;
        KEY key;
        key.key = id;
        key.addr = new;
        insert(key);

        srch = new;
    }
    else if(mode == 1) srch->mode = 1;
    return srch;
}

void update(int nbuckets, int* cnt, struct node* n, struct node* head, struct node* tail, int* hit_cnt, int* write_cnt){
    if(n->next != NULL){
        *hit_cnt += 1;
        n->hit += 1;
        return;
    }
    n->next = head->next;
    head->next->prev = n;
    head->next = n;
    n->prev = head;

    if(*cnt > nbuckets){
        struct node* d = tail->prev;
        d->prev->next = tail;
        tail->prev = d->prev;
        if(d->mode == 1){
            *write_cnt += 1;
        }
        DelNode(d->id);
        free(d);
        *cnt -= 1;
        evict_cnt++;
    }
}

int main(int argc, char *argv[]){
    char* inputFileName = argv[1]; //실행시 입력
    int nbuckets = atoi(argv[2]);

    struct node* head = (struct node*)malloc(sizeof(struct node));
    struct node* tail = (struct node*)malloc(sizeof(struct node));
    head->id = -1;
    head->prev = NULL;
    head->next = tail;
    tail->id = -1;
    tail->prev = head;
    tail->next = NULL;

    int cnt = 0; //현재 linked list의 노드 번호
    int idx = 0; //몇번째로 들어온 lba인지
    int hit_cnt = 0;
    int read_cnt = 0;
    int write_cnt = 0;

    FILE* fp = fopen(inputFileName, "r");

    while(!feof(fp)){
        long lba;
        char rw;
        int mode = 0;
        fscanf(fp,"%c %ld\n",&rw,&lba);
        if(rw == 'W'){
            mode = 1;
        }
        struct node* n = find(&cnt, lba, idx, mode, &read_cnt);
        update(nbuckets, &cnt, n, head, tail, &hit_cnt, &write_cnt);
        idx += 1;
    }
    fclose(fp);

    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read: %d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
    printf("evict: %d, read evict: %d\n", evict_cnt, evict_cnt - write_cnt);
}
