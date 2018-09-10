#pragma once
#ifndef __LIST_H
#define __LIST_H

#define LIST(type) struct type *
#define LIST_ENTRY(type) struct type *__next
#define LIST_OBJ(type) struct type *

#define LIST_ADD(list, entry)\
	if ((entry) != NULL) {\
		if ((list) != NULL) {\
			(entry)->__next = (list);\
		}\
		(list) = (entry);\
	}

#define LIST_REMOVE(type, entry, list)\
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

#define LIST_POP_FIRST(list)\
	list;\
	if ((list) != NULL) {\
		list = (list)->__next;\
	}

// Iterate through a list. Unsafe if list is modified during iteration.
#define LIST_FOREACH(type, var, list)\
	for (struct type *var = (list); var != NULL; var = var->__next)

// Iterate through a list, safe version. Use the BEGIN macro at the beginning of the loop body.
#define LIST_FOREACH_SAFE(type, var, list)\
	for (struct type *var = (list), *__tmp = NULL; var != NULL; var = __tmp)

#define LIST_FOREACH_SAFE_BEGIN(var)\
	__tmp = var->__next

#define LIST_FOREACH_SAFE_NEXT()\
	__tmp

#define LIST_IS_FIRST(list, var)\
	((list) == var)

#define LIST_IS_LAST(var)\
	(var->__next == NULL)

#define LIST_IS_EMPTY(list)\
	((list) == NULL)

#endif
