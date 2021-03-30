#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>


typedef struct node {
	struct node *next;
	int time;
	char event;
	int processID1;
	int running;
	int ready;
	int blocked;
} node;

typedef struct queue {
	node *front;
	node *rear;
} queue;

void enqueue(queue*, int, char*, int);
void dequeue(queue*);
void clean(queue*);


void enqueue(queue* q, int time, char *event, int p_id1) {
	node *new_node = (node *) malloc(sizeof(node));

	new_node->time = time;
	new_node->running = 0;
	new_node->ready = 0;
	new_node->blocked = 0;
	new_node->event = *event;
	new_node->processID1 = p_id1;

	if (q->front != NULL) {
		q->rear->next = new_node;
		q->rear = new_node;
		q->rear->next = NULL;
	} else {
		q->front = new_node;
		q->rear = new_node;
		q->front->next = q->rear->next = NULL;
	}

}

void insert_node(queue* q, node* qnode) {

	if (q->front != NULL) {
		q->rear->next = qnode;
		q->rear = qnode;
		q->rear->next = NULL;
	} else {
		q->front = qnode;
		q->rear = qnode;
		q->front->next = q->rear->next = NULL;
	}

}

void dequeue(queue* q) {

	node *ptr = NULL;

	if (q->front) {
		ptr = q->front;
		if (q->front == q->rear) {
			printf("dod\n");
			q->front = NULL;
			q->rear = NULL;
		} else {
			q->front = ptr->next;
		}
	}

}

node *dequeue1(queue *q) {
	node *ptr = NULL;
	if (q->front) {
		ptr = q->front;
		if (q->front == q->rear) {
			q->front = NULL;
			q->rear = NULL;
		} else {
			q->front = ptr->next;
		}
	}
	return ptr;
}


void clean(queue* q) {

	while (q->front != NULL) {
		node *temp;
		node *p = q->front;

		while (p != NULL) {
			temp = p;
			p = p->next;
			free(temp);
		}

		q->front = NULL;
		q->rear = NULL;

	}
}

char *trim(char *str) {
	char *end;

	// Trim leading space
	while (isspace((unsigned char) *str))
		str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char) *end))
		end--;

	// Write new null terminator character
	end[1] = '\0';

	return str;
}

// main function
int main(void) {

	queue *running = (queue *) malloc(sizeof(queue));
	running->front = NULL;
	running->rear = NULL;
	queue *ready = (queue *) malloc(sizeof(queue));
	ready->front = NULL;
	ready->rear = NULL;
	queue *exited = (queue *) malloc(sizeof(queue));
	exited->front = NULL;
	exited->rear = NULL;

	node *block_list[10];
	int block_num = 0;

	node *exit_list[10];
	int exit_num = 0;

	int time;
	char *event;
	int p_id1 = 0;
	int p_id2 = 0;

	// System idle initialize
	int sys_idle_id = 0;
	node *sys_idle = (node *) malloc(sizeof(node));
	sys_idle->next = NULL;
	sys_idle->processID1 = sys_idle_id;
	sys_idle->time = 0;

	char *token;

	char input[100];
	int i = 0;

	while (*(fgets(input, sizeof(input), stdin)) != '\n') {

//		fgets(input, sizeof(input), stdin);

		token = strtok(input, " ");

		while (token != NULL) {
			i += 1;
			if (i == 1) {
				time = atoi(token);
			} else if (i == 2) {
				event = trim(token);
			} else if (i == 3) {
				p_id1 = atoi(token);
			} else if (i == 4) {
				p_id2 = atoi(token);
			}
			token = strtok(NULL, " ");
		}

		if (strcmp(event, "C") == 0) {
			if (running->front == NULL || running->front->processID1 == 0) {
				dequeue(running);
				enqueue(running, time, event, p_id1);
				sys_idle->running = sys_idle->running + (time - sys_idle->time);
				sys_idle->time = time;
			} else {
				enqueue(ready, time, event, p_id1);
			}
			
		}

		if (strcmp(event, "T") == 0) {
			node* current_run = running->front;
			current_run->running = current_run->running
					+ (time - current_run->time);
			current_run->time = time;

			insert_node(ready, current_run);

			dequeue(running);

			if (ready->front != NULL) {
				node* new_run = dequeue1(ready);
				new_run->ready = new_run->ready + (time - new_run->time);
				new_run->time = time;
				printf("%d", ready->front->processID1);
				insert_node(running, new_run);
			}

			printf("running: %d ready: %d\n", running->front->processID1,
					ready->front->running);
		}

		if (strcmp(event, "R") == 0) {
			node* current_run = running->front;
			current_run->running = current_run->running
					+ (time - current_run->time);
			current_run->time = time;
			block_list[block_num] = current_run;
			block_num = block_num + 1;
			dequeue(running);

			if (ready->front != NULL) {
				node* new_run = dequeue1(ready);
				new_run->ready = new_run->ready + (time - new_run->time);
				new_run->time = time;
				insert_node(running, new_run);
			} else{
				insert_node(running, sys_idle);
			}
//			printf(" R running: %d ready: %d", running->front->running,
//					ready->rear->ready);
		}

		if (strcmp(event, "I") == 0) {
			node *unblock;
			for (int j = 0; j < block_num; j++) {
				if (block_list[j]->processID1 == p_id2) {
					unblock = block_list[j];
					for (int k = j; k < block_num; k++) {
						block_list[k] = block_list[k + 1];
					}
					unblock->blocked = unblock->blocked
							+ (time - unblock->time);
					unblock->time = time;
					if(running->front == NULL || running->front->processID1 == 0){
						dequeue(running);
						insert_node(running, unblock);
						if(running->front->processID1 == 0){
							sys_idle->running = sys_idle->running + (time - sys_idle->time);
							sys_idle->time = time;
						}
					} else{
						insert_node(ready, unblock);
					}
					block_num = block_num - 1;

					break;
				}
			}
//			printf(" I running: %d ready: %d", running->front->running,
//					ready->rear->ready);
		}

		if (strcmp(event, "E") == 0) {
			if (running->front->processID1 == p_id1) {
				node* current_run = running->front;
				current_run->running = current_run->running
						+ (time - current_run->time);
				current_run->time = time;
				exit_list[exit_num] = current_run;
				dequeue(running);

				if (ready->front != NULL) {
					node* new_run = dequeue1(ready);
					new_run->ready = new_run->ready + (time - new_run->time);
					new_run->time = time;
					insert_node(running, new_run);
				}
//				printf("running: %d\n", running->front->processID1);
				exit_num = exit_num + 1;
			} else if (ready->front->processID1 == p_id1) {
				node* current_run = dequeue1(ready);
				current_run->running = current_run->running
						+ (time - current_run->time);
				current_run->time = time;
				exit_list[exit_num] = current_run;

				exit_num = exit_num + 1;
			}
//			printf(" E running: %d ready: %d", running->front->running,
//					ready->rear->ready);
		}

		i = 0;

	}
	
	exit_list[0] = sys_idle;
	
	int counter = 0;
	int good = 0;
	for (int p = 0; p < exit_num; p++) {
		
		while(good == 0){
			if(exit_list[p]->processID1 == counter){
				printf("running: %d ready: %d blocked: %d\n", exit_list[p]->running,
								exit_list[p]->ready, exit_list[p]->blocked);
				counter ++;
			}
		}
		

	}


}

