#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"BTREE.h"

int nbuckets;
int cnt = 0; //현재 linked list의 노드 번호
int idx = 0; //몇번째로 들어온 lba인지
FILE* fp;
struct node* head;
struct node* tail;
int hit_cnt = 0;

int read_cnt = 0;
int write_cnt = 0;
int evict_cnt = 0;

struct node* find(long id, int mode, int idx){
    struct node* srch = search(id);
    if(srch == NULL){
        struct node* new = (struct node*)malloc(sizeof(struct node));
        new->id = id;
        new->prev = NULL;
        new->next = NULL;
        new->mode = mode;
        new->hit = 0;
        cnt += 1;
        read_cnt += 1;

        KEY key;
        key.key = id;
        key.addr = new;
        insert(key);

        srch = new;
    }
    else if(mode == 1) srch->mode = 1;
    return srch;
}

void update(struct node* n, int idx){
    if(n->next != NULL){
        n->prev->next = n->next;
	    n->next->prev = n->prev;
        hit_cnt += 1;
        n->hit += 1;
    }

    n->next = head->next;
    head->next->prev = n;
    head->next = n;
    n->prev = head;

    if(cnt > nbuckets){
        struct node* d = tail->prev;
        d->prev->next = tail;
        tail->prev = d->prev;
        if(d->mode == 1){
            write_cnt += 1;
        }
        DelNode(d->id);
        free(d);
        cnt -= 1;
		evict_cnt += 1;
    }
}


int main(int argc, char *argv[]){
    char* inputFileName = argv[1]; //실행시 입력
    nbuckets = atoi(argv[2]);
    
    head = (struct node*)malloc(sizeof(struct node));
    tail = (struct node*)malloc(sizeof(struct node));
    head->id = -1;
    head->prev = NULL;
    head->next = tail;
    tail->id = -1;
    tail->prev = head;
    tail->next = NULL;

    fp = fopen(inputFileName, "r");
    
    
    while(!feof(fp)){
        long lba;
        char rw;
        int mode = 0;
        if(!feof(fp)){
            fscanf(fp,"%c %ld\n", &rw, &lba);
        }
        else{
            break;
        }
        if(rw == 'W'){
            mode = 1;
        }
        struct node* n = find(lba, mode, idx);
        update(n, idx);
        idx += 1;
    }
    fclose(fp);

    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read:%d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
    printf("evict: %d, read evict: %d\n", evict_cnt, evict_cnt - write_cnt);
}
