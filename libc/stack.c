// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <stack.h>

static int nonce = 0;

struct stack *stack_new(void)
{
	struct stack *stack = malloc(sizeof(*stack));
	stack->tail = NULL;
	return stack;
}

void stack_destroy(struct stack *stack)
{
	struct stack_node *iterator = stack->tail;
	while (iterator) {
		if (!iterator->prev) {
			free(iterator);
			break;
		}
		iterator = iterator->prev;
		free(iterator->next);
	}
	stack->tail = NULL;
	free(stack);
	stack = NULL;
}

static struct stack_node *stack_new_node(void)
{
	struct stack_node *node = malloc(sizeof(*node));
	node->data = NULL;
	node->prev = NULL;
	node->next = NULL;
	node->nonce = nonce++;
	return node;
}

static u32 stack_push_bot_node(struct stack *stack, struct stack_node *node)
{
	if (!stack || !node)
		return 0;

	if (stack->tail) {
		struct stack_node *iterator = stack->tail;
		while (iterator) {
			if (!iterator->prev)
				break;
			iterator = iterator->prev;
		}
		iterator->prev = node;
		node->next = iterator;
	} else {
		stack->tail = node;
	}

	return 1;
}

static u32 stack_push_node(struct stack *stack, struct stack_node *node)
{
	if (!stack || !node)
		return 0;

	if (stack->tail) {
		stack->tail->next = node;
		node->prev = stack->tail;
		stack->tail = node;
	} else {
		stack->tail = node;
	}

	return 1;
}

u32 stack_empty(struct stack *stack)
{
	return !stack->tail;
}

u32 stack_push_bot(struct stack *stack, void *data)
{
	struct stack_node *node = stack_new_node();
	node->data = data;
	return stack_push_bot_node(stack, node);
}

u32 stack_push(struct stack *stack, void *data)
{
	struct stack_node *node = stack_new_node();
	node->data = data;
	return stack_push_node(stack, node);
}

void *stack_pop(struct stack *stack)
{
	if (!stack || !stack->tail)
		return NULL;

	struct stack_node *prev = stack->tail;

	stack->tail->prev->next = NULL;
	stack->tail = stack->tail->prev;

	void *data = prev->data;
	free(prev);
	return data;
}

void *stack_peek(struct stack *stack)
{
	if (!stack || !stack->tail)
		return NULL;

	return stack->tail->data;
}

void stack_clear(struct stack *stack)
{
	while (stack_pop(stack))
		;
}
