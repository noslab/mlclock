#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"BTREE.h"

int nbuckets;
int cnt = 0; //현재 linked list의 노드 번호
int cnt_fifo = 0;
int idx = 0; //몇번째로 들어온 lba인지
FILE* fp;
struct node* head;
struct node* tail;
struct node* head_lfu;
struct node* tail_lfu;

int hit_cnt = 0;
int read_cnt = 0;
int write_cnt = 0;
int evict_cnt = 0;

struct node* find(long id, int mode){
    struct node* srch = search(id);
    if(srch == NULL){
        struct node* new = (struct node*)malloc(sizeof(struct node));
        new->id = id;
        new->prev = NULL;
        new->next = NULL;
        new->mode = mode;
        new->hit = 0;
        cnt += 1;
        cnt_fifo += 1;
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

void update(struct node* n){
    if(n->next != NULL){
        if(n->hit == 0){
            n->prev->next = n->next;
            n->next->prev = n->prev;

            n->next = head_lfu->next;
            n->prev = head_lfu;
            n->prev->next = n;
            n->next->prev = n;
        }
        
        n->hit++;
        hit_cnt++;

        return;
    }

    n->prev = head;
    n->next = head->next;
    head->next = n;
    n->next->prev = n;

    if(cnt > nbuckets){
        struct node* d;
        if(cnt_fifo < 2){
            d = head_lfu->next;
            struct node* find = d;
            int MIN = d->hit;
            while(find != tail_lfu){
                if(find->hit <= MIN){
                    d = find;
                    MIN = find->hit;
                }
                find = find->next;
            }
        }
        else{
            d = tail->prev;
            cnt_fifo -= 1;
        }
        d->prev->next = tail;
        tail->prev = d->prev;
        if(d->mode == 1){
            write_cnt += 1;
        }
        DelNode(d->id);
        free(d);
        cnt -= 1;
		evict_cnt++;
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

    head_lfu = (struct node*)malloc(sizeof(struct node));
    tail_lfu = (struct node*)malloc(sizeof(struct node));
    head_lfu->id = -1;
    head_lfu->prev = NULL;
    head_lfu->next = tail_lfu;
    tail_lfu->id = -1;
    tail_lfu->prev = head_lfu;
    tail_lfu->next = NULL;

    fp = fopen(inputFileName, "r");

    while(!feof(fp)){
        long lba;
        char rw;
        int mode = 0;

        fscanf(fp,"%c %ld\n", &rw, &lba);

        if(rw == 'W'){
            mode = 1;
        }
        struct node* n = find(lba, mode);
        update(n);

        idx += 1;
    }
    fclose(fp);

    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read: %d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
    printf("evict: %d, read evict: %d\n", evict_cnt, evict_cnt - write_cnt);
}
