#pragma once
#ifndef __STACK_H
#define __STACK_H

/*
====================================================================================================

	Stack

	A very simple last-in, first-out data collection which is embedded into the collection members
	for minimizing memory allocations.

====================================================================================================
*/

#define stack_t(type) struct type *
#define stack_entry(type) struct type *__next

// -------------------------------------------------------------------------------------------------

#define stack_push(list, entry)\
	if ((entry) != NULL) {\
		if ((list) != NULL) {\
			(entry)->__next = (list);\
		}\
		else  {\
			(entry)->__next = NULL;\
		}\
		(list) = (entry);\
	}

#define stack_remove(type, entry, list)\
	if ((entry) != (list)) {\
		for (struct type *__tmp = (list), *__prev_tmp = NULL; __tmp != NULL; __prev_tmp = __tmp, __tmp = __tmp->__next) {\
			if (__tmp == (entry)) {\
				__prev_tmp->__next = __tmp->__next;\
				break;\
			}\
		}\
	}\
	else {\
		(list) = (entry)->__next;\
	}

#define stack_pop_first(list)\
	(list);\
	if ((list) != NULL) {\
		(list) = (list)->__next;\
	}

// Iterate through a stack. Unsafe if stack is modified during iteration.
#define stack_foreach(type, var, list)\
	for (struct type *var = (list); var != NULL; var = var->__next)

// Iterate through a stack, safe version. Use with the 'stack_foreach_safe_begin' macro at the
// beginning of the loop body.
#define stack_foreach_safe(type, var, list)\
	for (struct type *var = (list), *__tmp = NULL; var != NULL; var = __tmp)

#define stack_foreach_safe_begin(var)\
	__tmp = var->__next

#define stack_foreach_safe_next()\
	__tmp

#define stack_next(item)\
	((item)->__next)

#define stack_first(list)\
	(list)

#define stack_is_first(list, var)\
	((list) == var)

#define stack_is_last(var)\
	(var->__next == NULL)

#define stack_is_empty(list)\
	((list) == NULL)

#endif
