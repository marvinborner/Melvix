// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STACK_H
#define STACK_H

#include <def.h>

struct stack_node {
	void *data;
	int nonce;
	struct stack_node *next;
	struct stack_node *prev;
};

struct stack {
	struct stack_node *tail;
};

struct stack *stack_new();
void stack_destroy(struct stack *stack);
u32 stack_empty(struct stack *stack);
u32 stack_push_bot(struct stack *stack, void *data);
u32 stack_push(struct stack *stack, void *data);
void *stack_pop(struct stack *stack);
void *stack_peek(struct stack *stack);

#endif
