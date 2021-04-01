// Inspired by sxml (capmar)
// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef XML_H
#define XML_H

#include <def.h>

enum xml_error {
	XML_ERROR_INVALID = -1,
	XML_SUCCESS = 0,
	XML_ERROR_BUFFERDRY = 1,
	XML_ERROR_TOKENSFULL = 2
};

struct xml_token {
	u16 type;
	u16 size;
	u32 start_pos;
	u32 end_pos;
};

struct xml_args {
	const char *buffer;
	u32 buffer_length;
	struct xml_token *tokens;
	u32 num_tokens;
};

enum xml_type {
	XML_START_TAG,
	XML_END_TAG,
	XML_CHARACTER,
	XML_CDATA,
	XML_INSTRUCTION,
	XML_DOCTYPE,
	XML_COMMENT
};

struct xml {
	u32 buffer_pos;
	u32 ntokens;
	u32 tag_level;
};

enum xml_error xml_parse(struct xml *parser, const char *buffer, u32 buffer_length,
			 struct xml_token *tokens, u32 num_tokens) NONNULL;

void xml_init(struct xml *parser) NONNULL;

#endif
