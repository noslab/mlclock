#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<time.h>
#include "BTREE.h"

int nbuckets;
int cnt = 0;
int evict_cnt = 0;

struct node* hand = NULL;

int find(int id, int mode, int idx, struct node* hand){
    struct node* srch = search(id);
    if(srch == NULL)
        return 0;
    else{
        srch->ref = 1;
        if(mode == 1)
			srch->mode = mode;
        return 1;
    }
}

void update(int idx, int* read_cnt, int* write_cnt){
    if(hand->mode != -1){
        struct node* start = hand;
        while(1){
            if(hand->ref == 0)
				break;
			hand->ref -= 1;
            hand = hand->next;
        }
        DelNode(hand->id);
		evict_cnt++;
    }
    *read_cnt += 1;
    if(hand->mode == 1) *write_cnt += 1;
	
	struct node* d = hand;
    hand = hand->next;

	d->next->prev = d->prev;
	d->prev->next = d->next;
	free(d);
	cnt--;
}

void display_clock(){
	struct node* tmp = hand;
	while(1){
		printf("%d, ",tmp->id);
		tmp = tmp->next;
		if(tmp == hand)
			break;
	}
	printf("\n");
}

void insert_new(int id, int idx, int mode){
	struct node* new = (struct node*)malloc(sizeof(struct node));
	new->id = id;
	new->ref = 0;
	new->mode = mode;

	KEY key;
	key.key = id;
	key.addr = new;
	insert(key);
	
	if(hand != NULL){
		struct node* tmp = search_next(id);
		if(tmp->id < id){
			new->prev = tmp;
			tmp->next->prev = new;
			new->next = tmp->next;
			tmp->next = new;
		}
		else{
			new->next = tmp;
			tmp->prev->next = new;
			new->prev = tmp->prev;
			tmp->prev = new;
		}
	}
	else{
		hand = new;
		new->prev = new;
		new->next = new;
	}

	cnt++;
}

int main(int argc, char *argv[]){
    char* inputFileName = argv[1]; //실행시 입력
    nbuckets = atoi(argv[2]);

    int idx = 0; //몇번째로 들어온 lba인지
    int hit_cnt = 0;
    int read_cnt = 0;
    int write_cnt = 0;

    FILE* fp = fopen(inputFileName, "r");

    while(!feof(fp)){
        if(idx % 1000000 == 0)
            printf("%d\n", idx);
        int id;
        char rw;
        int mode = 0;
        float pred = nbuckets;
        fscanf(fp,"%c %d\n",&rw,&id);
        if(rw == 'W'){
            mode = 1;
        }
        int hit_check = find(id, mode, idx, hand);
        hit_cnt += hit_check;
        if(hit_check == 0){
			if(cnt == nbuckets){
            	update(idx, &read_cnt, &write_cnt);
			}
			insert_new(id, idx, mode);
		}
        idx += 1;
    }
    fclose(fp);

    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read: %d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
    printf("evict: %d, read evict: %d\n", evict_cnt, evict_cnt - write_cnt);
}
