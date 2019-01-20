#pragma once
#ifndef __LIST_H
#define __LIST_H

/*
====================================================================================================

	List

	A doubly-linked list which is embedded into the collection members.

====================================================================================================
*/

#define list_t(type) struct {\
	type *first;\
	type *last;\
}

#define list_entry(type) struct {\
	struct type *next;\
	struct type *previous;\
	void *list_reference;\
} __entry

#define list_init() { NULL, NULL }
#define list_entry_init() { NULL, NULL, NULL }

// -------------------------------------------------------------------------------------------------

#define list_push(list, item)\
	if ((list).first == NULL) {\
		(list).first = (item);\
		(list).last = (item);\
		(item)->__entry.previous = NULL;\
		(item)->__entry.next = NULL;\
	}\
	else {\
		(list).last->__entry.next = (item);\
		(item)->__entry.previous = (list).last;\
		(item)->__entry.next = NULL;\
		(list).last = (item);\
	}\
	(item)->__entry.list_reference = &(list);

#define list_remove(list, item)\
	if (list_contains((list), (item))) {\
		if ((item) == (list).first)\
			(list).first = (item)->__entry.next;\
		if ((item) == (list).last)\
			(list).last = (item)->__entry.previous;\
		if ((item)->__entry.previous)\
			(item)->__entry.previous->__entry.next = (item)->__entry.next;\
		if ((item)->__entry.next)\
			(item)->__entry.next->__entry.previous = (item)->__entry.previous;\
		(item)->__entry.list_reference = NULL;\
	}

#define list_foreach(list, var)\
	for ((var) = (list).first; (var); (var) = (var)->__entry.next)

#define list_foreach_safe(list, var, tmp)\
	for (\
		(var) = (list).first, (tmp) = ((var) ? (var)->__entry.next : NULL);\
		(var);\
		(var) = (tmp), (tmp) = ((var) ? (var)->__entry.next : NULL))

#define list_clear(list)\
	list.first = NULL;\
	list.last = NULL;

#define list_contains(list, item)\
	((item)->__entry.list_reference == &(list))

#endif
