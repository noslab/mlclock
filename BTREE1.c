/*Edited from https://www.goeduhub.com/9916/c-program-for-implementation-of-b-tree*/
#include "BTREE1.h"

node* insert(node* root, KEY key) {
        node *newnode;
        KEY upKey;
        KeyStatus value;
        value = ins(root, key, &upKey, &newnode);
        if (value == Duplicate)
                printf("Key already available\n");
        if (value == InsertIt) {
                node *uproot = root;
                root = (node*)malloc(sizeof(node));
                root->n = 1;
                root->keys[0] = upKey;
                root->p[0] = uproot;
                root->p[1] = newnode;
        }/*End of if */
		return root;
}/*End of insert()*/

KeyStatus ins(node *ptr, KEY key, KEY *upKey, node **newnode) {
        node *newPtr, *lastPtr;
        int pos, i, n, splitPos;
        KEY newKey, lastKey;
        KeyStatus value;
        if (ptr == NULL) {
                *newnode = NULL;
                *upKey = key;
                return InsertIt;
        }
        n = ptr->n;
        pos = searchPos(key.key, ptr->keys, n);
        if (pos < n && key.key == ptr->keys[pos].key)
                return Duplicate;
        value = ins(ptr->p[pos], key, &newKey, &newPtr);
        if (value != InsertIt)
                return value;
        /*If keys in node is less than M-1 where M is order of B tree*/
        if (n < M - 1) {
                pos = searchPos(newKey.key, ptr->keys, n);
                /*Shifting the key and pointer right for inserting the new key*/
                for (i = n; i>pos; i--) {
                        ptr->keys[i] = ptr->keys[i - 1];
                        ptr->p[i + 1] = ptr->p[i];
                }
                /*Key is inserted at exact location*/
                ptr->keys[pos] = newKey;
                ptr->p[pos + 1] = newPtr;
                ++ptr->n; /*incrementing the number of keys in node*/
                return Success;
        }/*End of if */
         /*If keys in nodes are maximum and position of node to be inserted is last*/
        if (pos == M - 1) {
                lastKey = newKey;
                lastPtr = newPtr;
        }
        else { /*If keys in node are maximum and position of node to be inserted is not last*/
                lastKey = ptr->keys[M - 2];
                lastPtr = ptr->p[M - 1];
                for (i = M - 2; i>pos; i--) {
                        ptr->keys[i] = ptr->keys[i - 1];
                        ptr->p[i + 1] = ptr->p[i];
                }
                ptr->keys[pos] = newKey;
                ptr->p[pos + 1] = newPtr;
        }
        splitPos = (M - 1) / 2;
        (*upKey) = ptr->keys[splitPos];

        (*newnode) = (node*)malloc(sizeof(node));/*Right node after split*/
        ptr->n = splitPos; /*No. of keys for left splitted node*/
        (*newnode)->n = M - 1 - splitPos;/*No. of keys for right splitted node*/
        for (i = 0; i < (*newnode)->n; i++) {
                (*newnode)->p[i] = ptr->p[i + splitPos + 1];
                if (i < (*newnode)->n - 1)
                        (*newnode)->keys[i] = ptr->keys[i + splitPos + 1];
                else
                        (*newnode)->keys[i] = lastKey;
        }
        (*newnode)->p[(*newnode)->n] = lastPtr;
        return InsertIt;
}/*End of ins()*/

void display(node *ptr, int blanks) {
        if (ptr) {
                int i;
                for (i = 1; i <= blanks; i++)
                        printf(" ");
                for (i = 0; i < ptr->n; i++)
                        printf("%d ", ptr->keys[i].key);
                printf("\n");
                for (i = 0; i <= ptr->n; i++)
                        display(ptr->p[i], blanks + 10);
        }/*End of if*/
}/*End of display()*/

struct node* search(node* root, int key) {
        int pos, i, n;
        node *ptr = root;
        //printf("Search path:\n");
        while (ptr) {
                n = ptr->n;
                //for (i = 0; i < ptr->n; i++)
                        //printf(" %d", ptr->keys[i]);
                //printf("\n");
                pos = searchPos(key, ptr->keys, n);
                if (pos < n && key == ptr->keys[pos].key) {
                        //printf("Key %d found in position %d of last dispalyed node\n", key, i);
                        return ptr->keys[pos].addr;
                }
                ptr = ptr->p[pos];
        }
        //printf("Key %d is not available\n", key);
        return NULL;
}/*End of search()*/

struct node* search_next(node* root, int key) {
		int pos, i, n;
		node *ptr = root;
		int chk = 0;
		struct node *ret = NULL;
		while (ptr) {
				n = ptr->n;
				if (chk == 0)
						pos = searchPos(key, ptr->keys, n);
				else if (chk == 1)
						pos = n;
				else
						pos = 0;
				if (pos < n && key == ptr->keys[pos].key) {
						if (n != 1 && pos != 0){
								ret = ptr->keys[pos-1].addr;
								chk = 1;
						}
						else if (n != 1 && pos == 0){
								ret = ptr->keys[pos+1].addr;
								ptr = ptr->p[pos+1];
								chk = 2;
								continue;
						}
				}
				else if (pos < n){
						ret = ptr->keys[pos].addr;
				}
				else {
						ret = ptr->keys[pos-1].addr;
				}
				ptr = ptr->p[pos];
		}
		return ret;
}

int searchPos(int key, KEY *key_arr, int n) {
        int pos = 0;
        while (pos < n && key > key_arr[pos].key)
                pos++;
        return pos;
}/*End of searchPos()*/

node* DelNode(node* root, int key) {
        node *uproot;
        KeyStatus value;
        value = del(root, root, key);
        switch (value) {
        case SearchFailure:
                printf("Key %d is not available\n", key);
                break;
        case LessKeys:
                uproot = root;
                root = root->p[0];
                free(uproot);
                break;
        }/*End of switch*/
		return root;
}/*End of delnode()*/

KeyStatus del(node* root, node *ptr, int key) {
        int pos, i, pivot, n, min;
        KEY *key_arr;
        KeyStatus value;
        node **p, *lptr, *rptr;

        if (ptr == NULL)
                return SearchFailure;
        /*Assigns values of node*/
        n = ptr->n;
        key_arr = ptr->keys;
        p = ptr->p;
        min = (M - 1) / 2;/*Minimum number of keys*/

                                          //Search for key to delete
        pos = searchPos(key, key_arr, n);
        // p is a leaf
        if (p[0] == NULL) {
                if (pos == n || key < key_arr[pos].key)
                        return SearchFailure;
                /*Shift keys and pointers left*/
                for (i = pos + 1; i < n; i++)
                {
                        key_arr[i - 1] = key_arr[i];
                        p[i] = p[i + 1];
                }
                return --ptr->n >= (ptr == root ? 1 : min) ? Success : LessKeys;
        }/*End of if */

         //if found key but p is not a leaf
        if (pos < n && key == key_arr[pos].key) {
                node *qp = p[pos], *qp1;
                int nkey;
                while (1) {
                        nkey = qp->n;
                        qp1 = qp->p[nkey];
                        if (qp1 == NULL)
                                break;
                        qp = qp1;
                }/*End of while*/
                key_arr[pos] = qp->keys[nkey - 1];
                qp->keys[nkey - 1].key = key;
        }/*End of if */
        value = del(root, p[pos], key);
        if (value != LessKeys)
                return value;

        if (pos > 0 && p[pos - 1]->n > min) {
                pivot = pos - 1; /*pivot for left and right node*/
                lptr = p[pivot];
                rptr = p[pos];
                /*Assigns values for right node*/
                rptr->p[rptr->n + 1] = rptr->p[rptr->n];
                for (i = rptr->n; i>0; i--) {
                        rptr->keys[i] = rptr->keys[i - 1];
                        rptr->p[i] = rptr->p[i - 1];
                }
                rptr->n++;
                rptr->keys[0] = key_arr[pivot];
                rptr->p[0] = lptr->p[lptr->n];
                key_arr[pivot] = lptr->keys[--lptr->n];
                return Success;
        }/*End of if */
         //if (posn > min)
        if (pos < n && p[pos + 1]->n > min) {
                pivot = pos; /*pivot for left and right node*/
                lptr = p[pivot];
                rptr = p[pivot + 1];
                /*Assigns values for left node*/
                lptr->keys[lptr->n] = key_arr[pivot];
                lptr->p[lptr->n + 1] = rptr->p[0];
                key_arr[pivot] = rptr->keys[0];
                lptr->n++;
                rptr->n--;
                for (i = 0; i < rptr->n; i++) {
                        rptr->keys[i] = rptr->keys[i + 1];
                        rptr->p[i] = rptr->p[i + 1];
                }/*End of for*/
                rptr->p[rptr->n] = rptr->p[rptr->n + 1];
                return Success;
        }/*End of if */

        if (pos == n)
                pivot = pos - 1;
        else
                pivot = pos;

        lptr = p[pivot];
        rptr = p[pivot + 1];
        /*merge right node with left node*/
        lptr->keys[lptr->n] = key_arr[pivot];
        lptr->p[lptr->n + 1] = rptr->p[0];
        for (i = 0; i < rptr->n; i++) {
                lptr->keys[lptr->n + 1 + i] = rptr->keys[i];
                lptr->p[lptr->n + 2 + i] = rptr->p[i + 1];
        }
        lptr->n = lptr->n + rptr->n + 1;
        free(rptr); /*Remove right node*/
        for (i = pos + 1; i < n; i++) {
                key_arr[i - 1] = key_arr[i];
                p[i] = p[i + 1];
        }
        return --ptr->n >= (ptr == root ? 1 : min) ? Success : LessKeys;
}/*End of del()*/

void eatline(void) {
        char c;
        while ((c = getchar()) != '\n');
}

/* Function to display each key in the tree in sorted order (in-order traversal)
@param struct node *ptr, the pointer to the node you are currently working with
*/
void inorder(node *ptr) {
        if (ptr) {
                if (ptr->n >= 1) {
                        inorder(ptr->p[0]);
                        printf("%d ", ptr->keys[0].key);
                        inorder(ptr->p[1]);
                        if (ptr->n == 2) {
                                printf("%d ", ptr->keys[1].key);
                                inorder(ptr->p[2]);
                        }
                }
        }
}

/* Function that returns the total number of keys in the tree.
@param struct node *ptr, the pointer to the node you are currently working with
*/
int totalKeys(node *ptr) {
        if (ptr) {
                int count = 1;
                if (ptr->n >= 1) {
                        count += totalKeys(ptr->p[0]);
                        count += totalKeys(ptr->p[1]);
                        if (ptr->n == 2) count += totalKeys(ptr->p[2]) + 1;
                }
                return count;
        }
        return 0;
}

/* Function that prints the total number of keys in the tree.
@param struct node *ptr, the pointer to the node you are currently working with
*/
void printTotal(node *ptr) {
        printf("%d\n", totalKeys(ptr));
}

/* Function that returns the smallest key found in the tree.
@param struct node *ptr, the pointer to the node you are currently working with
*/
int getMin(node *ptr) {
        if (ptr) {
                int min;
                if (ptr->p[0] != NULL) min = getMin(ptr->p[0]);
                else min = ptr->keys[0].key;
                return min;
        }
        return 0;
}

/* Function that returns the largest key found in the tree.
@param struct node *ptr, the pointer to the node you are currently working with
*/
int getMax(node *ptr) {
        if (ptr) {
                int max;
                if (ptr->n == 1) {
                        if (ptr->p[1] != NULL) max = getMax(ptr->p[1]);
                        else max = ptr->keys[0].key;
                }
                if (ptr->n == 2) {
                        if (ptr->p[2] != NULL) max = getMax(ptr->p[2]);
                        else max = ptr->keys[1].key;
                }
                return max;
        }
        return 0;
}

/* Function that prints the smallest and largest keys found in the tree.
@param struct node *ptr, the pointer to the node you are currently working with
*/
void getMinMax(node *ptr) {
        printf("%d %d\n", getMin(ptr), getMax(ptr));
}

/* Function that determines the largest number.
@param int, integer to compare.
@param int, integer to compare.
@param int, integer to compare.
*/
int max(int first, int second, int third) {
        int max = first;
        if (second > max) max = second;
        if (third > max) max = third;
        return max;
}

/*Function that finds the maximum level in the node and returns it as an integer.
@param struct node *ptr, the node to find the maximum level for.
*/
int maxLevel(node *ptr) {
        if (ptr) {
                int l = 0, mr = 0, r = 0, max_depth;
                if (ptr->p[0] != NULL) l = maxLevel(ptr->p[0]);
                if (ptr->p[1] != NULL) mr = maxLevel(ptr->p[1]);
                if (ptr->n == 2) {
                        if (ptr->p[2] != NULL) r = maxLevel(ptr->p[2]);
                }
                max_depth = max(l, mr, r) + 1;
                return max_depth;
        }
        return 0;
}

/*Function that prints the maximum level in the tree.
@param struct node *ptr, the tree to find the maximum level for.
*/
void printMaxLevel(node *ptr) {
        int max = maxLevel(ptr) - 1;
        if (max == -1) printf("tree is empty\n");
        else printf("%d\n", max);
}
