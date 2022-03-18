#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "BTREE.h"

int nbuckets;
int evict_cnt = 0;

int find(int id, int mode, int idx, struct node* hand, int* read_cnt){
    struct node* srch = search(id);
    if(srch == NULL){
        *read_cnt += 1;
        return 0;
    }
    else{
        srch->ref = 1;
        srch->mode = mode;
        srch->hit += 1;
        return 1;
    }
}

struct node* update(int id, int mode, int idx, struct node* hand, int* write_cnt){
    if(hand->mode != -1){
        while(hand->ref > 0){
            hand->ref -= 1;
            hand = hand->next;
        }
        DelNode(hand->id);
		evict_cnt++;
    }
    if(hand->mode == 1) *write_cnt += 1;

    hand->id = id;
    hand->hit = 0;
    hand->mode = mode;

    KEY key;
    key.key = id;
    key.addr = hand;
    insert(key);

    hand = hand->next;
    return hand;
}

int main(int argc, char *argv[]){
    char* inputFileName = argv[1]; //실행시 입력
    nbuckets = atoi(argv[2]);

    struct node* hand = NULL;
    struct node* tmp = NULL;
    for(int i = 0;i < nbuckets;i++){
        struct node* new = (struct node*)malloc(sizeof(struct node));
        new->id = -1;
        new->mode = -1;
        new->hit = 0;
        new->ref = 0;
        if(hand == NULL){
            hand = new;
            tmp = new;
        }
        else{
            tmp->next = new;
            tmp = new;
        }
    }
    tmp->next = hand;

    int idx = 0; //몇번째로 들어온 lba인지
    int hit_cnt = 0;
    int read_cnt = 0;
    int write_cnt = 0;

    FILE* fp = fopen(inputFileName, "r");

    while(!feof(fp)){
        int id;
        char rw;
        int mode = 0;
        float pred = nbuckets;

        fscanf(fp,"%c %d\n",&rw,&id);
        if(rw == 'W'){
            mode = 1;
        }

        int hit_check = find(id, mode, idx, hand, &read_cnt);
        hit_cnt += hit_check;
        if(hit_check == 0){
            hand = update(id, mode, idx, hand, &write_cnt);
        }
        idx += 1;
    }
    fclose(fp);
  
    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read:%d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
    printf("evict: %d, read evict: %d\n", evict_cnt, evict_cnt - write_cnt);
}
