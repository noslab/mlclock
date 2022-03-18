#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<time.h>
#include "BTREE1.h"

int nbuckets;
int cnt = 0;
int gh_cnt = 0;

int false_cnt = 0;

int evict_cnt = 0;
int ml_evict = 0;

struct node* w_hand = NULL;
struct node* r_hand = NULL;
struct node* gh_head = NULL;
struct node* gh_tail = NULL;

node* r_root = NULL;
node* w_root = NULL;
node* gh_root = NULL;

float w_d = -0.5;
float w_h = 0.5;
float bias = 0.5;

float lr = 0.05;

int write_cnt = 0;
int read_cnt = 0;

float traintime = 0;
float predicttime = 0;

void evict_gh(struct node* new);
void insert_new(struct node* new);
void evict(int idx);

void w_update(float distance, int hit, int answer, float pred){
    struct timeval start, end;
    gettimeofday(&start, NULL);
    w_d = w_d + lr * distance * (answer - pred);
    w_h = w_h + lr * hit * (answer - pred);
    bias = bias + lr * (answer - pred);
    gettimeofday(&end, NULL);
    traintime += (end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
}

float predict(float distance, int hit){
    float y = 0;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    y += bias;
    y += w_h * hit;
    y += w_d * distance;
    gettimeofday(&end, NULL);
    predicttime += (end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    return y;
}

int find(int id, int mode, int idx){
	int gh = 0;
	struct node* srch = search(r_root, id);
	if(srch == NULL) srch = search(w_root, id);
	if(srch == NULL){
		gh = 1;
		srch = search(gh_root, id);
	}
	
    if(srch == NULL){
		if(cnt == nbuckets)
			evict(idx);
		struct node* new = (struct node*)malloc(sizeof(struct node));
		new->id = id;
		new->idx = idx;
		new->hit = 0;
		new->ref = 0;
		new->mode = mode;
		insert_new(new);

		read_cnt++;
		cnt++;
        return 0;
	}
	else if(gh == 1){
		if(srch->mode == -1) false_cnt++;
		if(predict((float)srch->idx / nbuckets, srch->hit) < 0){
			w_update((float)srch->idx / nbuckets, srch->hit, 1, 0);
		}
			
		srch->idx = idx;
		srch->hit = 0;
		srch->ref = 0;
		srch->mode = mode;
		//printf("%d %d\n", srch->next == NULL ? 0 : 1, srch->next == gh_tail ? 0 : 1);
		if(srch != gh_head && srch != gh_tail){
			srch->prev->next = srch->next;
			srch->next->prev = srch->prev;
		}
		else if(srch != gh_tail){ //srch가 head
			gh_head = srch->next;
			gh_head->prev = NULL;
		}
		else if(srch != gh_head){ //srch가 tail
			gh_tail = srch->prev;
			gh_tail->next = NULL;
		}
		else{
			gh_head = NULL;
			gh_tail = NULL;
		}
		gh_root = DelNode(gh_root, srch->id);
		gh_cnt--;

		if(cnt == nbuckets)
			evict(idx);

		insert_new(srch);
		
		read_cnt++;
		cnt++;
		return 0;
	}
    else{
        if(predict((float)(idx - srch->idx) / nbuckets, srch->hit) < 0){
            w_update((float)(idx - srch->idx) / nbuckets, srch->hit, 1, 0);
        }
        srch->idx = idx;
        srch->hit += 1;
		srch->ref = 1;
        if(mode == 1 && srch->mode == 0){
			srch->mode = 1;
			if(srch == r_hand){
				r_hand = srch->next;
				if(r_hand == srch)
					r_hand = NULL;
			}			

			srch->prev->next = srch->next;
			srch->next->prev = srch->prev;
			
			r_root = DelNode(r_root, srch->id);
			
			insert_new(srch);
		}
		return 1;
    }
}

void evict(int idx){
	evict_cnt++;
    struct node* victim = NULL;
	if(r_hand != NULL && w_hand != NULL){
		while(r_hand->ref != 0){
			r_hand->ref -= 1;
			r_hand = r_hand->next;
		}
		while(w_hand->ref != 0){
			w_hand->ref -= 1;
			w_hand = w_hand->next;
		}

		if(predict((float)(idx - r_hand->idx) / nbuckets, r_hand->hit) < 0){
			victim = r_hand;
			r_hand->mode = -1;
			ml_evict++;
		}
		else if(predict((float)(idx - w_hand->idx) / nbuckets, w_hand->hit) < 0){
			victim = w_hand;
			w_hand->mode = -1;
			ml_evict++;
		}
		else{
			victim = r_hand;
		}
    }
	else if(r_hand == NULL){
		while(w_hand->ref != 0){
			w_hand->ref -= 1;
			w_hand = w_hand->next;
		}
		victim = w_hand;
	}
	else{
		while(r_hand->ref != 0){
			r_hand->ref -= 1;
			r_hand = r_hand->next;
		}
		victim = r_hand;
	}
	
	if(victim == r_hand){
		r_hand = r_hand->next;
		if(r_hand == victim)
			r_hand = NULL;
		r_root = DelNode(r_root, victim->id);
		if(w_hand != NULL)
			w_hand->ref = 1;
	}
	else{
		w_hand = w_hand->next;
		if(w_hand == victim)
			w_hand = NULL;
		w_root = DelNode(w_root, victim->id);
	}
    if(victim->mode == 1) write_cnt += 1;
	
	victim->next->prev = victim->prev;
	victim->prev->next = victim->next;
	
	victim->idx = idx - victim->idx;
	cnt--;

	evict_gh(victim);
}

void evict_gh(struct node* new){
	new->prev = NULL;
	new->next = NULL;

	KEY key;
	key.key = new->id;
	key.addr = new;
	gh_root = insert(gh_root, key);

	if(gh_head == NULL){
		gh_head = new;
		gh_tail = new;
	}
	else{
		new->next = gh_head;
		gh_head->prev = new;
		gh_head = new;
	}

	gh_cnt++;

	if(gh_cnt > nbuckets){
		struct node* victim = gh_tail;
		gh_tail = gh_tail->prev;
		gh_tail->next = NULL;
		
		if(predict((float)victim->idx / nbuckets, victim->hit) >= 0)
			w_update((float)victim->idx / nbuckets, victim->hit, 0, 1);
		gh_cnt--;
		
		gh_root = DelNode(gh_root, victim->id);
		free(victim);
	}
}

void insert_new(struct node* new){
	KEY key;
	key.key = new->id;
	key.addr = new;

	if(new->mode == 0){
		if(r_hand != NULL){
			new->next = r_hand;
			r_hand->prev->next = new;
			new->prev = r_hand->prev;
			r_hand->prev = new;
		}
		else{
			r_hand = new;
			new->prev = new;
			new->next = new;
		}
		r_root = insert(r_root, key);
	}
	else{
		w_root = insert(w_root,key);
		if(w_hand != NULL){
			struct node* tmp = search_next(w_root, new->id);
			if(tmp->id < new->id){
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
			w_hand = new;
			new->prev = new;
			new->next = new;
		}
	}
}

int main(int argc, char *argv[]){
    char* inputFileName = argv[1]; //실행시 입력
    nbuckets = atoi(argv[2]);

    int idx = 0; //몇번째로 들어온 lba인지
	int hit_cnt = 0;

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
        int hit_check = find(id, mode, idx);
		hit_cnt += hit_check;
        idx += 1;
    }
    fclose(fp);
   

    printf("lines: %d\n", idx);
    printf("hit counts: %d\n", hit_cnt);
    printf("hit ratio: %lf\n", (double)hit_cnt/(double)idx);
    printf("read: %d\nwrite: %d\nhit: %d\n", read_cnt, write_cnt, hit_cnt);
	printf("evict: %d, read evict: %d\n",evict_cnt, evict_cnt - write_cnt);
	printf("ml evict: %d, false: %d, ratio: %lf\n", ml_evict, false_cnt, (double)false_cnt/ml_evict);
    printf("train time: %f\n", traintime);
    printf("predict time: %f\n", predicttime);
}
