/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

/**
 * Stack protector
 */

void __stack_chk_fail(void);
NORETURN void __stack_chk_fail(void)
{
	panic("FATAL: Stack smashing detected");
}

void __stack_chk_fail_local(void);
NORETURN void __stack_chk_fail_local(void)
{
	panic("FATAL: Local stack smashing detected");
}

/**
 * UBSan
 */

#define is_aligned(value, alignment) !(value & (alignment - 1))

struct source_location {
	const char *file;
	u32 line;
	u32 column;
};

struct type_descriptor {
	u16 kind;
	u16 info;
	char name[];
};

struct type_mismatch {
	struct source_location location;
	struct type_descriptor *type;
	u8 alignment;
	u8 type_check_kind;
};

struct nonnull_arg {
	struct source_location location;
	struct source_location attribute_location;
	u32 index;
};

struct overflow {
	struct source_location location;
	struct type_descriptor *type;
};

struct pointer_overflow {
	struct source_location location;
};

struct out_of_bounds {
	struct source_location location;
	struct type_descriptor *left_type;
	struct type_descriptor *right_type;
};

void __ubsan_handle_load_invalid_value(void);
void __ubsan_handle_load_invalid_value(void)
{
	panic("UBSAN: load-invalid-value");
}

void __ubsan_handle_nonnull_arg(struct nonnull_arg *data);
void __ubsan_handle_nonnull_arg(struct nonnull_arg *data)
{
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: nonnull-arg [index: %d]", loc->file, loc->line, data->index);
}

void __ubsan_handle_nullability_arg(struct nonnull_arg *data);
void __ubsan_handle_nullability_arg(struct nonnull_arg *data)
{
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: nonnull-arg [index: %d]", loc->file, loc->line, data->index);
}

void __ubsan_handle_nonnull_return_v1(void);
void __ubsan_handle_nonnull_return_v1(void)
{
	panic("UBSAN: nonnull-return-v1");
}

void __ubsan_handle_nullability_return_v1(void);
void __ubsan_handle_nullability_return_v1(void)
{
	panic("UBSAN: nullability-return-v1");
}

void __ubsan_handle_vla_bound_not_positive(void);
void __ubsan_handle_vla_bound_not_positive(void)
{
	panic("UBSAN: vla-bound-not-positive");
}

void __ubsan_handle_add_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_add_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: add-overflow [type: %s]", loc->file, loc->line, data->type->name);
}

void __ubsan_handle_sub_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_sub_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: sub-overflow [type: %s]", loc->file, loc->line, data->type->name);
}

void __ubsan_handle_negate_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_negate_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: negate-overflow [type: %s]", loc->file, loc->line, data->type->name);
}

void __ubsan_handle_mul_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_mul_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: mul-overflow [type: %s]", loc->file, loc->line, data->type->name);
}

void __ubsan_handle_shift_out_of_bounds(struct overflow *data, void *left, void *right);
void __ubsan_handle_shift_out_of_bounds(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: shift-out-of-bounds [type: %s]", loc->file, loc->line,
	      data->type->name);
}

void __ubsan_handle_divrem_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_divrem_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: divrem-overflow (probably div-by-zero) [type: %s]", loc->file,
	      loc->line, data->type->name);
}

void __ubsan_handle_out_of_bounds(struct out_of_bounds *data, void *value);
void __ubsan_handle_out_of_bounds(struct out_of_bounds *data, void *value)
{
	UNUSED(value);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: out-of-bounds", loc->file, loc->line);
}

void __ubsan_handle_type_mismatch_v1(struct type_mismatch *data, u32 ptr);
void __ubsan_handle_type_mismatch_v1(struct type_mismatch *data, u32 ptr)
{
	static const char *kinds[] = {
		"Load of",
		"Store to",
		"Reference binding to",
		"Member access within",
		"Member call on",
		"Constructor call on",
		"Downcast of",
		"Downcast of",
		"Upcast of",
		"Cast to virtual base of",
		"Nonnull binding to",
		"Dynamic operation on",
	};

	struct source_location *loc = &data->location;
	const char *msg;
	if (ptr == 0) {
		msg = "null pointer";
	} else if (data->alignment != 0 && is_aligned(ptr, data->alignment))
		msg = "misaligned memory address";
	else
		msg = "address with insufficient space";
	panic("%s:%d: UBSAN: %s %s [type: %s; addr: 0x%x; align: %d]", loc->file, loc->line,
	      kinds[data->type_check_kind], msg, data->type->name, ptr, data->alignment);
}

void __ubsan_handle_alignment_assumption(void);
void __ubsan_handle_alignment_assumption(void)
{
	panic("UBSAN: alignment-assumption");
}

void __ubsan_handle_builtin_unreachable(void);
void __ubsan_handle_builtin_unreachable(void)
{
	panic("UBSAN: builtin-unreachable");
}

void __ubsan_handle_missing_return(void);
void __ubsan_handle_missing_return(void)
{
	panic("UBSAN: missing-return");
}

void __ubsan_handle_implicit_conversion(void);
void __ubsan_handle_implicit_conversion(void)
{
	panic("UBSAN: implicit-conversion");
}

void __ubsan_handle_invalid_builtin(void);
void __ubsan_handle_invalid_builtin(void)
{
	panic("UBSAN: invalid-builtin");
}

void __ubsan_handle_pointer_overflow(struct pointer_overflow *data, void *value);
void __ubsan_handle_pointer_overflow(struct pointer_overflow *data, void *value)
{
	UNUSED(value);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: pointer-overflow", loc->file, loc->line);
}
