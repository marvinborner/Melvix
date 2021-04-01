// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <errno.h>
#include <mem.h>
#include <str.h>

u32 strlen(const char *str)
{
	const char *s = str;
	while (*s)
		s++;
	return s - str;
}

u32 strlcpy(char *dst, const char *src, u32 size)
{
	const char *orig = src;
	u32 left = size;

	if (left)
		while (--left)
			if (!(*dst++ = *src++))
				break;

	if (!left) {
		if (!size)
			*dst = 0;
		while (*src++)
			;
	}

	return src - orig - 1;
}

int strcmp(const char *s1, const char *s2)
{
	const u8 *c1 = (const u8 *)s1;
	const u8 *c2 = (const u8 *)s2;
	u8 ch;
	int d = 0;

	while (1) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

int strncmp(const char *s1, const char *s2, u32 n)
{
	const u8 *c1 = (const u8 *)s1;
	const u8 *c2 = (const u8 *)s2;
	u8 ch;
	int d = 0;

	while (n--) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

char *strchr(char *s, int c)
{
	while (*s != (char)c) {
		if (!*s)
			return NULL;
		s++;
	}

	return s;
}

char *strrchr(char *s, int c)
{
	char *ret = 0;

	do {
		if (*s == c)
			ret = s;
	} while (*s++);

	return ret;
}

u32 strlcat(char *dst, const char *src, u32 size)
{
	const char *orig_dst = dst;
	const char *orig_src = src;

	u32 n = size;
	while (n-- && *dst)
		dst++;

	u32 len = dst - orig_dst;
	n = size - len;

	if (!n--)
		return len + strlen(src);

	while (*src) {
		if (n) {
			*dst++ = *src;
			n--;
		}
		src++;
	}

	src = 0;

	return len + (src - orig_src);
}

char *strinv(char *s)
{
	u32 s_str = strlen(s);

	int iterations = (int)s_str / 2;
	for (int i = 0; i < iterations; i++) {
		char aux = s[i];
		s[i] = s[(s_str - i) - 1];
		s[(s_str - i) - 1] = aux;
	}
	return s;
}

char *strdup(const char *s)
{
	int l = strlen(s) + 1;
	char *d = malloc(l);

	memcpy(d, s, l);

	return d;
}

const char *strerror(u32 error)
{
	switch (error) {
	case 0:
		return "Success";
	case EPERM:
		return "Operation not permitted";
	case ENOENT:
		return "No such file or directory";
	case ESRCH:
		return "No such process";
	case EINTR:
		return "Interrupted system call";
	case EIO:
		return "I/O error";
	case ENXIO:
		return "No such device or address";
	case E2BIG:
		return "Argument list too long";
	case ENOEXEC:
		return "Exec format error";
	case EBADF:
		return "Bad file number";
	case ECHILD:
		return "No child processes";
	case EAGAIN:
		return "Try again";
	case ENOMEM:
		return "Out of memory";
	case EACCES:
		return "Permission denied";
	case EFAULT:
		return "Bad address";
	case ENOTBLK:
		return "Block device required";
	case EBUSY:
		return "Device or resource busy";
	case EEXIST:
		return "File exists";
	case EXDEV:
		return "Cross-device link";
	case ENODEV:
		return "No such device";
	case ENOTDIR:
		return "Not a directory";
	case EISDIR:
		return "Is a directory";
	case EINVAL:
		return "Invalid argument";
	case ENFILE:
		return "File table overflow";
	case EMFILE:
		return "Too many open files";
	case ENOTTY:
		return "Not a typewriter";
	case ETXTBSY:
		return "Text file busy";
	case EFBIG:
		return "File too large";
	case ENOSPC:
		return "No space left on device";
	case ESPIPE:
		return "Illegal seek";
	case EROFS:
		return "Read-only file system";
	case EMLINK:
		return "Too many links";
	case EPIPE:
		return "Broken pipe";
	case EDOM:
		return "Math argument out of domain of func";
	case ERANGE:
		return "Math result not representable";
	case EMAX:
		return "Max errno";
	default:
		return "Unknown error";
	}
}
