// MIT License, Copyright (c) 2021 Marvin Borner
// Detect stack overflows and other bugs

#include <def.h>
#include <print.h>

/**
 * Stack protector
 */

#define STACK_CHK_GUARD 0xdeadbeef

u32 __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void);
void __stack_chk_fail(void)
{
	panic("FATAL: Stack smashing detected\n");
}

void __stack_chk_fail_local(void);
void __stack_chk_fail_local(void)
{
	panic("FATAL: Local stack smashing detected\n");
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
	u32 alignment;
	u8 type_check_kind;
};

struct overflow {
	struct source_location location;
	struct type_descriptor *type;
};

struct out_of_bounds {
	struct source_location location;
	struct type_descriptor *left_type;
	struct type_descriptor *right_type;
};

void __ubsan_handle_load_invalid_value(void);
void __ubsan_handle_load_invalid_value(void)
{
	panic("UBSAN: load-invalid-value\n");
}

void __ubsan_handle_nonnull_arg(void);
void __ubsan_handle_nonnull_arg(void)
{
	panic("UBSAN: nonnull-arg\n");
}

void __ubsan_handle_nullability_arg(void);
void __ubsan_handle_nullability_arg(void)
{
	panic("UBSAN: nullability-arg\n");
}

void __ubsan_handle_nonnull_return_v1(void);
void __ubsan_handle_nonnull_return_v1(void)
{
	panic("UBSAN: nonnull-return-v1\n");
}

void __ubsan_handle_nullability_return_v1(void);
void __ubsan_handle_nullability_return_v1(void)
{
	panic("UBSAN: nullability-return-v1\n");
}

void __ubsan_handle_vla_bound_not_positive(void);
void __ubsan_handle_vla_bound_not_positive(void)
{
	panic("UBSAN: vla-bound-not-positive\n");
}

void __ubsan_handle_add_overflow(void);
void __ubsan_handle_add_overflow(void)
{
	panic("UBSAN: add-overflow\n");
}

void __ubsan_handle_sub_overflow(void);
void __ubsan_handle_sub_overflow(void)
{
	panic("UBSAN: sub-overflow\n");
}

void __ubsan_handle_negate_overflow(void);
void __ubsan_handle_negate_overflow(void)
{
	panic("UBSAN: negate-overflow\n");
}

void __ubsan_handle_mul_overflow(void);
void __ubsan_handle_mul_overflow(void)
{
	panic("UBSAN: mul-overflow\n");
}

void __ubsan_handle_shift_out_of_bounds(void);
void __ubsan_handle_shift_out_of_bounds(void)
{
	panic("UBSAN: shift-out-of-bounds\n");
}

void __ubsan_handle_divrem_overflow(struct overflow *data, void *left, void *right);
void __ubsan_handle_divrem_overflow(struct overflow *data, void *left, void *right)
{
	UNUSED(left);
	UNUSED(right);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: divrem-overflow\n", loc->file, loc->line);
}

void __ubsan_handle_out_of_bounds(struct out_of_bounds *data, void *value);
void __ubsan_handle_out_of_bounds(struct out_of_bounds *data, void *value)
{
	UNUSED(value);
	struct source_location *loc = &data->location;
	panic("%s:%d: UBSAN: out-of-bounds\n", loc->file, loc->line);
}

void __ubsan_handle_type_mismatch_v1(struct type_mismatch *data, u32 ptr);
void __ubsan_handle_type_mismatch_v1(struct type_mismatch *data, u32 ptr)
{
	struct source_location *loc = &data->location;
	const char *msg = "";
	if (ptr == 0) {
		msg = "Null pointer access";
	} else if (data->alignment != 0 && is_aligned(ptr, data->alignment))
		msg = "Misaligned memory access";
	else
		msg = "Insufficient space";
	panic("%s:%d: UBSAN: type-mismatch-v1: %s [type: %s]\n", loc->file, loc->line, msg,
	      data->type->name);
}

void __ubsan_handle_alignment_assumption(void);
void __ubsan_handle_alignment_assumption(void)
{
	panic("UBSAN: alignment-assumption\n");
}

void __ubsan_handle_builtin_unreachable(void);
void __ubsan_handle_builtin_unreachable(void)
{
	panic("UBSAN: builtin-unreachable\n");
}

void __ubsan_handle_missing_return(void);
void __ubsan_handle_missing_return(void)
{
	panic("UBSAN: missing-return\n");
}

void __ubsan_handle_implicit_conversion(void);
void __ubsan_handle_implicit_conversion(void)
{
	panic("UBSAN: implicit-conversion\n");
}

void __ubsan_handle_invalid_builtin(void);
void __ubsan_handle_invalid_builtin(void)
{
	panic("UBSAN: invalid-builtin\n");
}

void __ubsan_handle_pointer_overflow(void);
void __ubsan_handle_pointer_overflow(void)
{
	panic("UBSAN: pointer-overflow\n");
}
