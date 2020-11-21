// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef DNS_H
#define DNS_H

#include <def.h>

#define DNS_NOERROR 0
#define DNS_FORMERR 1
#define DNS_SERVFAIL 2
#define DNS_NXDOMAIN 3
#define DNS_NOTIMP 4
#define DNS_REFUSED 5
#define DNS_YXDOMAIN 6
#define DNS_XRRSET 7
#define DNS_NOTAUTH 8
#define DNS_NOTZONE 9

u32 dns_request(const char *name);

#endif
