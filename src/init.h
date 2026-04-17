#ifndef INIT_H
#define INIT_H

extern volatile void *___;

#define ENSURE_INIT (___ = (void *)0)

#endif