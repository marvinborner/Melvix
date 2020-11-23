// Inspired by sxml (capmar)
// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <mem.h>
#include <str.h>
#include <xml.h>

static const char *str_findchr(const char *start, const char *end, int c)
{
	const char *it;

	assert(start <= end);
	assert(0 <= c && c <= 127);

	it = (const char *)memchr((void *)start, c, end - start);
	return (it != NULL) ? it : end;
}

static const char *str_findstr(const char *start, const char *end, const char *needle)
{
	u32 needlelen;
	int first;
	assert(start <= end);

	needlelen = strlen(needle);
	assert(0 < needlelen);
	first = (u8)needle[0];

	while (start + needlelen <= end) {
		const char *it =
			(const char *)memchr((void *)start, first, (end - start) - (needlelen - 1));
		if (it == NULL)
			break;

		if (memcmp(it, needle, needlelen) == 0)
			return it;

		start = it + 1;
	}

	return end;
}

static int str_starts_with(const char *start, const char *end, const char *prefix)
{
	long nbytes;
	assert(start <= end);

	nbytes = strlen(prefix);
	if (end - start < nbytes)
		return 0;

	return memcmp(prefix, start, nbytes) == 0;
}

static int white_space(int c)
{
	switch (c) {
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	}

	return 0;
}

static int name_start_char(int c)
{
	if (0x80 <= c)
		return 1;

	return c == ':' || ('A' <= c && c <= 'Z') || c == '_' || ('a' <= c && c <= 'z');
}

static int name_char(int c)
{
	return name_start_char(c) || c == '-' || c == '.' || ('0' <= c && c <= '9') || c == 0xB7 ||
	       (0x0300 <= c && c <= 0x036F) || (0x203F <= c && c <= 0x2040);
}

#define is_space(c) (white_space(((u8)(c))))
#define is_alpha(c) (name_start_char(((u8)(c))))
#define is_alnum(c) (name_char(((u8)(c))))

static const char *str_ltrim(const char *start, const char *end)
{
	const char *it;
	assert(start <= end);

	for (it = start; it != end && is_space(*it); it++)
		;

	return it;
}

static const char *str_rtrim(const char *start, const char *end)
{
	const char *it, *prev;
	assert(start <= end);

	for (it = end; start != it; it = prev) {
		prev = it - 1;
		if (!is_space(*prev))
			return it;
	}

	return start;
}

static const char *str_find_notalnum(const char *start, const char *end)
{
	const char *it;
	assert(start <= end);

	for (it = start; it != end && is_alnum(*it); it++)
		;

	return it;
}

#define buffer_from_offset(args, i) ((args)->buffer + (i))
#define buffer_tooffset(args, ptr) (unsigned)((ptr) - (args)->buffer)
#define buffer_getend(args) ((args)->buffer + (args)->buffer_length)

static int state_push_token(struct xml *state, struct xml_args *args, enum xml_type type,
			    const char *start, const char *end)
{
	struct xml_token *token;
	u32 i;
	if (args->num_tokens <= state->ntokens)
		return 0;

	i = state->ntokens++;
	token = &args->tokens[i];
	token->type = type;
	token->start_pos = buffer_tooffset(args, start);
	token->end_pos = buffer_tooffset(args, end);
	token->size = 0;

	switch (type) {
	case XML_START_TAG:
		state->tag_level++;
		break;

	case XML_END_TAG:
		assert(0 < state->tag_level);
		state->tag_level--;
		break;

	default:
		break;
	}

	return 1;
}

static enum xml_error state_set_pos(struct xml *state, const struct xml_args *args, const char *ptr)
{
	state->buffer_pos = buffer_tooffset(args, ptr);
	return (state->ntokens <= args->num_tokens) ? XML_SUCCESS : XML_ERROR_TOKENSFULL;
}

#define state_commit(dest, src) memcpy((dest), (src), sizeof(struct xml))

#define XML_ERROR_STRICT XML_ERROR_INVALID
#define ENTITY_MAXLEN 8
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static enum xml_error parse_characters(struct xml *state, struct xml_args *args, const char *end)
{
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *limit, *colon, *ampr = str_findchr(start, end, '&');
	assert(end <= buffer_getend(args));

	if (ampr != start)
		state_push_token(state, args, XML_CHARACTER, start, ampr);

	if (ampr == end)
		return state_set_pos(state, args, ampr);

	limit = MIN(ampr + ENTITY_MAXLEN, end);
	colon = str_findchr(ampr, limit, ';');
	if (colon == limit)
		return (limit == end) ? XML_ERROR_BUFFERDRY : XML_ERROR_INVALID;

	start = colon + 1;
	state_push_token(state, args, XML_CHARACTER, ampr, start);
	return state_set_pos(state, args, start);
}

static enum xml_error parse_attrvalue(struct xml *state, struct xml_args *args, const char *end)
{
	while (buffer_from_offset(args, state->buffer_pos) != end) {
		enum xml_error err = parse_characters(state, args, end);
		if (err != XML_SUCCESS)
			return err;
	}

	return XML_SUCCESS;
}

static enum xml_error parse_attributes(struct xml *state, struct xml_args *args)
{
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	const char *name = str_ltrim(start, end);

	u32 ntokens = state->ntokens;
	assert(0 < ntokens);

	while (name != end && is_alpha(*name)) {
		const char *eq, *space, *quot, *value;
		enum xml_error err;

		eq = str_findchr(name, end, '=');
		if (eq == end)
			return XML_ERROR_BUFFERDRY;

		space = str_rtrim(name, eq);
		state_push_token(state, args, XML_CDATA, name, space);

		quot = str_ltrim(eq + 1, end);
		if (quot == end)
			return XML_ERROR_BUFFERDRY;
		else if (*quot != '\'' && *quot != '"')
			return XML_ERROR_INVALID;

		value = quot + 1;
		quot = str_findchr(value, end, *quot);
		if (quot == end)
			return XML_ERROR_BUFFERDRY;

		state_set_pos(state, args, value);
		err = parse_attrvalue(state, args, quot);
		if (err != XML_SUCCESS)
			return err;

		name = str_ltrim(quot + 1, end);
	}

	{
		struct xml_token *token = args->tokens + (ntokens - 1);
		token->size = (u16)(state->ntokens - ntokens);
	}

	return state_set_pos(state, args, name);
}

#define TAG_LEN(str) (sizeof(str) - 1)
#define TAG_MINSIZE 1

static enum xml_error parse_comment(struct xml *state, struct xml_args *args)
{
	static const char START_TAG[] = "<!--";
	static const char END_TAG[] = "-->";

	const char *dash;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	if (end - start < (int)TAG_LEN(START_TAG))
		return XML_ERROR_BUFFERDRY;

	if (!str_starts_with(start, end, START_TAG))
		return XML_ERROR_INVALID;

	start += TAG_LEN(START_TAG);
	dash = str_findstr(start, end, END_TAG);
	if (dash == end)
		return XML_ERROR_BUFFERDRY;

	state_push_token(state, args, XML_COMMENT, start, dash);
	return state_set_pos(state, args, dash + TAG_LEN(END_TAG));
}

static enum xml_error parse_instruction(struct xml *state, struct xml_args *args)
{
	static const char START_TAG[] = "<?";
	static const char END_TAG[] = "?>";

	enum xml_error err;
	const char *quest, *space;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	assert(TAG_MINSIZE <= end - start);

	if (!str_starts_with(start, end, START_TAG))
		return XML_ERROR_INVALID;

	start += TAG_LEN(START_TAG);
	space = str_find_notalnum(start, end);
	if (space == end)
		return XML_ERROR_BUFFERDRY;

	state_push_token(state, args, XML_INSTRUCTION, start, space);

	state_set_pos(state, args, space);
	err = parse_attributes(state, args);
	if (err != XML_SUCCESS)
		return err;

	quest = buffer_from_offset(args, state->buffer_pos);
	if (end - quest < (int)TAG_LEN(END_TAG))
		return XML_ERROR_BUFFERDRY;

	if (!str_starts_with(quest, end, END_TAG))
		return XML_ERROR_INVALID;

	return state_set_pos(state, args, quest + TAG_LEN(END_TAG));
}

static enum xml_error parse_doctype(struct xml *state, struct xml_args *args)
{
	static const char START_TAG[] = "<!DOCTYPE";
	static const char END_TAG[] = ">";

	const char *bracket;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	if (end - start < (int)TAG_LEN(START_TAG))
		return XML_ERROR_BUFFERDRY;

	if (!str_starts_with(start, end, START_TAG))
		return XML_ERROR_BUFFERDRY;

	start += TAG_LEN(START_TAG);
	bracket = str_findstr(start, end, END_TAG);
	if (bracket == end)
		return XML_ERROR_BUFFERDRY;

	state_push_token(state, args, XML_DOCTYPE, start, bracket);
	return state_set_pos(state, args, bracket + TAG_LEN(END_TAG));
}

static enum xml_error parse_start(struct xml *state, struct xml_args *args)
{
	enum xml_error err;
	const char *gt, *name, *space;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	assert(TAG_MINSIZE <= end - start);

	if (!(start[0] == '<' && is_alpha(start[1])))
		return XML_ERROR_INVALID;

	name = start + 1;
	space = str_find_notalnum(name, end);
	if (space == end)
		return XML_ERROR_BUFFERDRY;

	state_push_token(state, args, XML_START_TAG, name, space);

	state_set_pos(state, args, space);
	err = parse_attributes(state, args);
	if (err != XML_SUCCESS)
		return err;

	gt = buffer_from_offset(args, state->buffer_pos);

	if (gt != end && *gt == '/') {
		state_push_token(state, args, XML_END_TAG, name, space);
		gt++;
	}

	if (gt == end)
		return XML_ERROR_BUFFERDRY;

	if (*gt != '>')
		return XML_ERROR_INVALID;

	return state_set_pos(state, args, gt + 1);
}

static enum xml_error parse_end(struct xml *state, struct xml_args *args)
{
	const char *gt, *space;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	assert(TAG_MINSIZE <= end - start);

	if (!(str_starts_with(start, end, "</") && is_alpha(start[2])))
		return XML_ERROR_INVALID;

	start += 2;
	gt = str_findchr(start, end, '>');
	if (gt == end)
		return XML_ERROR_BUFFERDRY;

	space = str_find_notalnum(start, gt);
	if (str_ltrim(space, gt) != gt)
		return XML_ERROR_STRICT;

	state_push_token(state, args, XML_END_TAG, start, space);
	return state_set_pos(state, args, gt + 1);
}

static enum xml_error parse_cdata(struct xml *state, struct xml_args *args)
{
	static const char START_TAG[] = "<![CDATA[";
	static const char END_TAG[] = "]]>";

	const char *bracket;
	const char *start = buffer_from_offset(args, state->buffer_pos);
	const char *end = buffer_getend(args);
	if (end - start < (int)TAG_LEN(START_TAG))
		return XML_ERROR_BUFFERDRY;

	if (!str_starts_with(start, end, START_TAG))
		return XML_ERROR_INVALID;

	start += TAG_LEN(START_TAG);
	bracket = str_findstr(start, end, END_TAG);
	if (bracket == end)
		return XML_ERROR_BUFFERDRY;

	state_push_token(state, args, XML_CDATA, start, bracket);
	return state_set_pos(state, args, bracket + TAG_LEN(END_TAG));
}

void xml_init(struct xml *state)
{
	state->buffer_pos = 0;
	state->ntokens = 0;
	state->tag_level = 0;
}

#define ROOT_FOUND(state) (0 < (state)->tag_level)
#define ROOT_PARSED(state) ((state)->tag_level == 0)

enum xml_error xml_parse(struct xml *state, const char *buffer, u32 buffer_length,
			 struct xml_token tokens[], u32 num_tokens)
{
	struct xml temp = *state;
	const char *end = buffer + buffer_length;

	struct xml_args args;
	args.buffer = buffer;
	args.buffer_length = buffer_length;
	args.tokens = tokens;
	args.num_tokens = num_tokens;

	while (!ROOT_FOUND(&temp)) {
		enum xml_error err;
		const char *start = buffer_from_offset(&args, temp.buffer_pos);
		const char *lt = str_ltrim(start, end);
		state_set_pos(&temp, &args, lt);
		state_commit(state, &temp);

		if (end - lt < TAG_MINSIZE)
			return XML_ERROR_BUFFERDRY;

		if (*lt != '<')
			return XML_ERROR_INVALID;

		switch (lt[1]) {
		case '?':
			err = parse_instruction(&temp, &args);
			break;
		case '!':
			err = (lt[2] == '-') ? parse_comment(&temp, &args) :
						     parse_doctype(&temp, &args);
			break;
		default:
			err = parse_start(&temp, &args);
			break;
		}

		if (err != XML_SUCCESS)
			return err;

		state_commit(state, &temp);
	}

	while (!ROOT_PARSED(&temp)) {
		enum xml_error err;
		const char *start = buffer_from_offset(&args, temp.buffer_pos);
		const char *lt = str_findchr(start, end, '<');
		while (buffer_from_offset(&args, temp.buffer_pos) != lt) {
			err = parse_characters(&temp, &args, lt);
			if (err != XML_SUCCESS)
				return err;

			state_commit(state, &temp);
		}

		// TODO: Only for self-closing tags
		if (end - lt == 0)
			break;

		if (end - lt < TAG_MINSIZE)
			return XML_ERROR_BUFFERDRY;

		switch (lt[1]) {
		case '?':
			err = parse_instruction(&temp, &args);
			break;
		case '/':
			err = parse_end(&temp, &args);
			break;
		case '!':
			err = (lt[2] == '-') ? parse_comment(&temp, &args) :
						     parse_cdata(&temp, &args);
			break;
		default:
			err = parse_start(&temp, &args);
			break;
		}

		if (err != XML_SUCCESS)
			return err;

		state_commit(state, &temp);
	}

	return XML_SUCCESS;
}
