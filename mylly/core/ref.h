#pragma once
#ifndef __REFPTR_H
#define __REFPTR_H

#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

struct ref_t {
	int count;
	void (*destructor)(void *);
};

#define ref_counted() struct ref_t __ref

// -------------------------------------------------------------------------------------------------

#define ref_init(obj, destr_method) {\
	(obj)->__ref.count = 1;\
	(obj)->__ref.destructor = (destr_method);\
}

#define ref_inc(obj) ((obj)->__ref.count++, (obj))
#define ref_dec(obj) if ((--(obj)->__ref.count) == 0) { (obj)->__ref.destructor(obj); }
#define ref_dec_safe(obj) if ((obj) && (--(obj)->__ref.count) == 0) { (obj)->__ref.destructor(obj); }

#endif
