/*Edited from https://www.goeduhub.com/9916/c-program-for-implementation-of-b-tree*/
#include "stdio.h"
#include "stdlib.h"

#define M 3

struct node{
    int id;
    //int first;
    int idx;
    int ref;
    int mode;
    int hit;
    struct node* prev;
    struct node* next;
};

typedef struct _KEY {
    int key;
    struct node* addr;
} KEY;

typedef struct _node {
        int    n; /* n < M No. of keys in node will always less than order of B tree */
        KEY              keys[M - 1]; /*array of keys*/
        struct _node *p[M]; /* (n+1 pointers will be in use) */
} node;

typedef enum KeyStatus {
        Duplicate,
        SearchFailure,
        Success,
        InsertIt,
        LessKeys,
} KeyStatus;

void insert(KEY key);
void display(node *root, int);
void DelNode(int x);
struct node* search(int x);
struct node* search_next(int x);
KeyStatus ins(node *r, KEY x, KEY* y, node** u);
int searchPos(int x, KEY *key_arr, int n);
KeyStatus del(node *r, int x);
void eatline(void);
void inorder(node *ptr);
int totalKeys(node *ptr);
void printTotal(node *ptr);
int getMin(node *ptr);
int getMax(node *ptr);
void getMinMax(node *ptr);
int max(int first, int second, int third);
int maxLevel(node *ptr);
void printMaxLevel(node *ptr);
