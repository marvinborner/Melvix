/*
uPNG -- derived from LodePNG version 20100808

Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch
Copyright (c) 2020 Marvin Borner

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
		1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
		2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
		3. This notice may not be removed or altered from any source
		distribution.
*/

#include <assert.h>
#include <def.h>
#include <mem.h>
#include <png.h>
#include <sys.h>

#define MAKE_BYTE(b) ((b)&0xFF)
#define MAKE_DWORD(a, b, c, d)                                                                     \
	((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I', 'H', 'D', 'R')
#define CHUNK_IDAT MAKE_DWORD('I', 'D', 'A', 'T')
#define CHUNK_IEND MAKE_DWORD('I', 'E', 'N', 'D')

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

#define NUM_DEFLATE_CODE_SYMBOLS                                                                   \
	288 // 256 literals, the end code, some length codes, and 2 unused codes
#define NUM_DISTANCE_SYMBOLS 32 // The distance codes have their own symbols, 30 used, 2 unused
#define NUM_CODE_LENGTH_CODES                                                                      \
	19 // The code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros
#define MAX_SYMBOLS 288 // Largest number of symbols used by any tree type

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15 // Largest bitlen used by any tree type

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

#define SET_ERROR(png, code)                                                                       \
	do {                                                                                       \
		(png)->error = (code);                                                             \
		(png)->error_line = __LINE__;                                                      \
	} while (0)

#define png_chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define png_chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define png_chunk_critical(chunk) (((chunk)[4] & 32) == 0)

struct huffman_tree {
	u32 *tree2d;
	u32 maxbitlen; // Maximum number of bits a single code can get
	u32 numcodes; // Number of symbols in the alphabet = number of codes
};

// The base lengths represented by codes 257-285
static const u32 LENGTH_BASE[29] = { 3,	 4,  5,	 6,   7,   8,	9,   10,  11, 13,
				     15, 17, 19, 23,  27,  31,	35,  43,  51, 59,
				     67, 83, 99, 115, 131, 163, 195, 227, 258 };

// The extra bits used by codes 257-285 (added to base length)
static const u32 LENGTH_EXTRA[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
				      2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };

// The base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)
static const u32 DISTANCE_BASE[30] = {
	1,   2,	  3,   4,   5,	 7,    9,    13,   17,	 25,   33,   49,   65,	  97,	 129,
	193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

// The extra bits of backwards distances (added to base)
static const u32 DISTANCE_EXTRA[30] = { 0, 0, 0, 0, 1, 1, 2, 2,	 3,  3,	 4,  4,	 5,  5,	 6,
					6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

// The order in which "code length alphabet code lengths" are stored, out of this the huffman tree of the dynamic huffman tree lengths is generated
static const u32 CLCL[NUM_CODE_LENGTH_CODES] = { 16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
						 11, 4,	 12, 3, 13, 2, 14, 1, 15 };

static const u32 FIXED_DEFLATE_CODE_TREE[NUM_DEFLATE_CODE_SYMBOLS * 2] = {
	289, 370, 290, 307, 546, 291, 561, 292, 293, 300, 294, 297, 295, 296, 0,   1,	2,   3,
	298, 299, 4,   5,   6,	 7,   301, 304, 302, 303, 8,   9,   10,	 11,  305, 306, 12,  13,
	14,  15,  308, 339, 309, 324, 310, 317, 311, 314, 312, 313, 16,	 17,  18,  19,	315, 316,
	20,  21,  22,  23,  318, 321, 319, 320, 24,  25,  26,  27,  322, 323, 28,  29,	30,  31,
	325, 332, 326, 329, 327, 328, 32,  33,	34,  35,  330, 331, 36,	 37,  38,  39,	333, 336,
	334, 335, 40,  41,  42,	 43,  337, 338, 44,  45,  46,  47,  340, 355, 341, 348, 342, 345,
	343, 344, 48,  49,  50,	 51,  346, 347, 52,  53,  54,  55,  349, 352, 350, 351, 56,  57,
	58,  59,  353, 354, 60,	 61,  62,  63,	356, 363, 357, 360, 358, 359, 64,  65,	66,  67,
	361, 362, 68,  69,  70,	 71,  364, 367, 365, 366, 72,  73,  74,	 75,  368, 369, 76,  77,
	78,  79,  371, 434, 372, 403, 373, 388, 374, 381, 375, 378, 376, 377, 80,  81,	82,  83,
	379, 380, 84,  85,  86,	 87,  382, 385, 383, 384, 88,  89,  90,	 91,  386, 387, 92,  93,
	94,  95,  389, 396, 390, 393, 391, 392, 96,  97,  98,  99,  394, 395, 100, 101, 102, 103,
	397, 400, 398, 399, 104, 105, 106, 107, 401, 402, 108, 109, 110, 111, 404, 419, 405, 412,
	406, 409, 407, 408, 112, 113, 114, 115, 410, 411, 116, 117, 118, 119, 413, 416, 414, 415,
	120, 121, 122, 123, 417, 418, 124, 125, 126, 127, 420, 427, 421, 424, 422, 423, 128, 129,
	130, 131, 425, 426, 132, 133, 134, 135, 428, 431, 429, 430, 136, 137, 138, 139, 432, 433,
	140, 141, 142, 143, 435, 483, 436, 452, 568, 437, 438, 445, 439, 442, 440, 441, 144, 145,
	146, 147, 443, 444, 148, 149, 150, 151, 446, 449, 447, 448, 152, 153, 154, 155, 450, 451,
	156, 157, 158, 159, 453, 468, 454, 461, 455, 458, 456, 457, 160, 161, 162, 163, 459, 460,
	164, 165, 166, 167, 462, 465, 463, 464, 168, 169, 170, 171, 466, 467, 172, 173, 174, 175,
	469, 476, 470, 473, 471, 472, 176, 177, 178, 179, 474, 475, 180, 181, 182, 183, 477, 480,
	478, 479, 184, 185, 186, 187, 481, 482, 188, 189, 190, 191, 484, 515, 485, 500, 486, 493,
	487, 490, 488, 489, 192, 193, 194, 195, 491, 492, 196, 197, 198, 199, 494, 497, 495, 496,
	200, 201, 202, 203, 498, 499, 204, 205, 206, 207, 501, 508, 502, 505, 503, 504, 208, 209,
	210, 211, 506, 507, 212, 213, 214, 215, 509, 512, 510, 511, 216, 217, 218, 219, 513, 514,
	220, 221, 222, 223, 516, 531, 517, 524, 518, 521, 519, 520, 224, 225, 226, 227, 522, 523,
	228, 229, 230, 231, 525, 528, 526, 527, 232, 233, 234, 235, 529, 530, 236, 237, 238, 239,
	532, 539, 533, 536, 534, 535, 240, 241, 242, 243, 537, 538, 244, 245, 246, 247, 540, 543,
	541, 542, 248, 249, 250, 251, 544, 545, 252, 253, 254, 255, 547, 554, 548, 551, 549, 550,
	256, 257, 258, 259, 552, 553, 260, 261, 262, 263, 555, 558, 556, 557, 264, 265, 266, 267,
	559, 560, 268, 269, 270, 271, 562, 565, 563, 564, 272, 273, 274, 275, 566, 567, 276, 277,
	278, 279, 569, 572, 570, 571, 280, 281, 282, 283, 573, 574, 284, 285, 286, 287, 0,   0
};

static const u32 FIXED_DISTANCE_TREE[NUM_DISTANCE_SYMBOLS * 2] = {
	33, 48, 34, 41, 35, 38, 36, 37, 0,  1,	2,  3,	39, 40, 4,  5,	6,  7,	42, 45, 43, 44,
	8,  9,	10, 11, 46, 47, 12, 13, 14, 15, 49, 56, 50, 53, 51, 52, 16, 17, 18, 19, 54, 55,
	20, 21, 22, 23, 57, 60, 58, 59, 24, 25, 26, 27, 61, 62, 28, 29, 30, 31, 0,  0
};

static u8 read_bit(u32 *bitpointer, const u8 *bitstream)
{
	u8 result = (u8)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
	(*bitpointer)++;
	return result;
}

static u32 read_bits(u32 *bitpointer, const u8 *bitstream, u32 nbits)
{
	u32 result = 0, i;
	for (i = 0; i < nbits; i++)
		result |= ((u32)read_bit(bitpointer, bitstream)) << i;
	return result;
}

// The buffer must be numcodes*2 in size!
static void huffman_tree_init(struct huffman_tree *tree, u32 *buffer, u32 numcodes, u32 maxbitlen)
{
	tree->tree2d = buffer;

	tree->numcodes = numcodes;
	tree->maxbitlen = maxbitlen;
}

// Given the code lengths (as stored in the PNG file), generate the tree as defined by deflate. maxbitlen is the maximum bits that a code in the tree can have
static void huffman_tree_create_lengths(struct png *png, struct huffman_tree *tree,
					const u32 *bitlen)
{
	u32 tree1d[MAX_SYMBOLS];
	u32 blcount[MAX_BIT_LENGTH];
	u32 nextcode[MAX_BIT_LENGTH + 1];
	u32 bits, n, i;
	u32 nodefilled = 0; // Up to which node it is filled
	u32 treepos = 0; // Position in the tree (1 of the numcodes columns)

	// Initialize local vectors
	memset(blcount, 0, sizeof(blcount));
	memset(nextcode, 0, sizeof(nextcode));

	// Step 1: Count number of instances of each code length
	for (bits = 0; bits < tree->numcodes; bits++) {
		blcount[bitlen[bits]]++;
	}

	// Step 2: Generate the nextcode values
	for (bits = 1; bits <= tree->maxbitlen; bits++) {
		nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
	}

	// Step 3: Generate all the codes
	for (n = 0; n < tree->numcodes; n++) {
		if (bitlen[n] != 0) {
			tree1d[n] = nextcode[bitlen[n]]++;
		}
	}

	/* Convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as many columns as codes - 1.
	   A good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what their 0 and 1 option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen */
	for (n = 0; n < tree->numcodes * 2; n++) {
		tree->tree2d[n] = 32767; // 32767 here means the tree2d isn't filled there yet
	}

	for (n = 0; n < tree->numcodes; n++) { // The codes
		for (i = 0; i < bitlen[n]; i++) { // The bits for this code
			u8 bit = (u8)((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
			// Check if oversubscribed
			if (treepos > tree->numcodes - 2) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}

			if (tree->tree2d[2 * treepos + bit] == 32767) { // Not yet filled in
				if (i + 1 == bitlen[n]) { // Last bit
					// Put the current code in it
					tree->tree2d[2 * treepos + bit] = n;
					treepos = 0;
				} else { // Put address of the next step in here, first that address has to be found of course (it's just nodefilled + 1)...
					nodefilled++;
					tree->tree2d[2 * treepos + bit] =
						nodefilled +
						tree->numcodes; // Addresses encoded with numcodes added to it
					treepos = nodefilled;
				}
			} else {
				treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
			}
		}
	}

	for (n = 0; n < tree->numcodes * 2; n++) {
		if (tree->tree2d[n] == 32767) {
			tree->tree2d[n] = 0; // Remove possible remaining 32767's
		}
	}
}

static u32 huffman_decode_symbol(struct png *png, const u8 *in, u32 *bp,
				 const struct huffman_tree *codetree, u32 inlength)
{
	u32 treepos = 0, ct;
	u8 bit;
	for (;;) {
		// Error: end of input memory reached without endcode
		if (((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength) {
			SET_ERROR(png, PNG_EMALFORMED);
			return 0;
		}

		bit = read_bit(bp, in);

		ct = codetree->tree2d[(treepos << 1) | bit];
		if (ct < codetree->numcodes) {
			return ct;
		}

		treepos = ct - codetree->numcodes;
		if (treepos >= codetree->numcodes) {
			SET_ERROR(png, PNG_EMALFORMED);
			return 0;
		}
	}
}

// Get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
static void get_tree_inflate_dynamic(struct png *png, struct huffman_tree *codetree,
				     struct huffman_tree *codetreeD,
				     struct huffman_tree *codelengthcodetree, const u8 *in, u32 *bp,
				     u32 inlength)
{
	u32 codelengthcode[NUM_CODE_LENGTH_CODES];
	u32 bitlen[NUM_DEFLATE_CODE_SYMBOLS];
	u32 bitlenD[NUM_DISTANCE_SYMBOLS];
	u32 n, hlit, hdist, hclen, i;

	// Make sure that length values that aren't filled in will be 0, or a wrong tree will be generated
	if ((*bp) >> 3 >= inlength - 2) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	// Clear bitlen arrays
	memset(bitlen, 0, sizeof(bitlen));
	memset(bitlenD, 0, sizeof(bitlenD));

	// Number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already
	hlit = read_bits(bp, in, 5) + 257;

	// Number of distance codes. Unlike the spec, the value 1 is added to it here already
	hdist = read_bits(bp, in, 5) + 1;

	// Number of code length codes. Unlike the spec, the value 4 is added to it here already
	hclen = read_bits(bp, in, 4) + 4;

	for (i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
		if (i < hclen) {
			codelengthcode[CLCL[i]] = read_bits(bp, in, 3);
		} else {
			codelengthcode[CLCL[i]] = 0;
		}
	}

	huffman_tree_create_lengths(png, codelengthcodetree, codelengthcode);

	if (png->error != PNG_EOK)
		return;

	// Use this tree to read the lengths for the tree that this function will return
	i = 0;
	while (i < hlit + hdist) {
		u32 code = huffman_decode_symbol(png, in, bp, codelengthcodetree, inlength);
		if (png->error != PNG_EOK) {
			break;
		}

		if (code <= 15) { // Length code
			if (i < hlit) {
				bitlen[i] = code;
			} else {
				bitlenD[i - hlit] = code;
			}
			i++;
		} else if (code == 16) { // Repeat previous
			u32 replength = 3; // Read in the 2 bits that indicate repeat length (3-6)
			u32 value; // Set value to the previous code

			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(png, PNG_EMALFORMED);
				break;
			}
			// Error, bit pointer jumps past memory
			replength += read_bits(bp, in, 2);

			if ((i - 1) < hlit) {
				value = bitlen[i - 1];
			} else {
				value = bitlenD[i - hlit - 1];
			}

			// Repeat this value in the next lengths
			for (n = 0; n < replength; n++) {
				// i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(png, PNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = value;
				} else {
					bitlenD[i - hlit] = value;
				}
				i++;
			}
		} else if (code == 17) { // Repeat "0" 3-10 times
			u32 replength = 3; // Read in the bits that indicate repeat length
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(png, PNG_EMALFORMED);
				break;
			}

			// Error, bit pointer jumps past memory
			replength += read_bits(bp, in, 3);

			// Repeat this value in the next lengths
			for (n = 0; n < replength; n++) {
				// Error: i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(png, PNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = 0;
				} else {
					bitlenD[i - hlit] = 0;
				}
				i++;
			}
		} else if (code == 18) { // Repeat "0" 11-138 times
			u32 replength = 11; // Read in the bits that indicate repeat length
			// Error, bit pointer jumps past memory
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(png, PNG_EMALFORMED);
				break;
			}

			replength += read_bits(bp, in, 7);

			// Repeat this value in the next lengths
			for (n = 0; n < replength; n++) {
				// i is larger than the amount of codes
				if (i >= hlit + hdist) {
					SET_ERROR(png, PNG_EMALFORMED);
					break;
				}
				if (i < hlit)
					bitlen[i] = 0;
				else
					bitlenD[i - hlit] = 0;
				i++;
			}
		} else {
			// Somehow an unexisting code appeared
			SET_ERROR(png, PNG_EMALFORMED);
			break;
		}
	}

	if (png->error == PNG_EOK && bitlen[256] == 0) {
		SET_ERROR(png, PNG_EMALFORMED);
	}

	// Generate code trees
	if (png->error == PNG_EOK) {
		huffman_tree_create_lengths(png, codetree, bitlen);
	}
	if (png->error == PNG_EOK) {
		huffman_tree_create_lengths(png, codetreeD, bitlenD);
	}
}

// Inflate a block with dynamic of fixed Huffman tree
static void inflate_huffman(struct png *png, u8 *out, u32 outsize, const u8 *in, u32 *bp, u32 *pos,
			    u32 inlength, u32 btype)
{
	u32 codetree_buffer[DEFLATE_CODE_BUFFER_SIZE];
	u32 codetreeD_buffer[DISTANCE_BUFFER_SIZE];
	u32 done = 0;

	struct huffman_tree codetree;
	struct huffman_tree codetreeD;

	if (btype == 1) {
		// Fixed trees
		huffman_tree_init(&codetree, (u32 *)FIXED_DEFLATE_CODE_TREE,
				  NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, (u32 *)FIXED_DISTANCE_TREE, NUM_DISTANCE_SYMBOLS,
				  DISTANCE_BITLEN);
	} else if (btype == 2) {
		// Dynamic trees
		u32 codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
		struct huffman_tree codelengthcodetree;

		huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS,
				  DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS,
				  DISTANCE_BITLEN);
		huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer,
				  NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);
		get_tree_inflate_dynamic(png, &codetree, &codetreeD, &codelengthcodetree, in, bp,
					 inlength);
	}

	while (done == 0) {
		u32 code = huffman_decode_symbol(png, in, bp, &codetree, inlength);
		if (png->error != PNG_EOK) {
			return;
		}

		if (code == 256) {
			// End code
			done = 1;
		} else if (code <= 255) {
			// Literal symbol
			if ((*pos) >= outsize) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}

			// Store output
			out[(*pos)++] = (u8)(code);
		} else if (code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX) {
			// Part 1: Get length base
			u32 length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];
			u32 codeD, distance, numextrabitsD;
			u32 start, forward, backward, numextrabits;

			// Part 2: Get extra bits and add the value of that to length
			numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

			// Error, bit pointer will jump past memory
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}
			length += read_bits(bp, in, numextrabits);

			// Part 3: Get distance code
			codeD = huffman_decode_symbol(png, in, bp, &codetreeD, inlength);
			if (png->error != PNG_EOK) {
				return;
			}

			// Invalid distance code (30-31 are never used)
			if (codeD > 29) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}

			distance = DISTANCE_BASE[codeD];

			// Part 4: Get extra bits from distance
			numextrabitsD = DISTANCE_EXTRA[codeD];

			// Error, bit pointer will jump past memory
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}

			distance += read_bits(bp, in, numextrabitsD);

			// Part 5: Fill in all the out[n] values based on the length and dist
			start = (*pos);
			backward = start - distance;

			if ((*pos) + length >= outsize) {
				SET_ERROR(png, PNG_EMALFORMED);
				return;
			}

			for (forward = 0; forward < length; forward++) {
				out[(*pos)++] = out[backward];
				backward++;

				if (backward >= start) {
					backward = start - distance;
				}
			}
		}
	}
}

static void inflate_uncompressed(struct png *png, u8 *out, u32 outsize, const u8 *in, u32 *bp,
				 u32 *pos, u32 inlength)
{
	u32 p;
	u32 len, nlen, n;

	// Go to first boundary of byte
	while (((*bp) & 0x7) != 0) {
		(*bp)++;
	}
	p = (*bp) / 8; //byte position

	// Read len (2 bytes) and nlen (2 bytes)
	if (p >= inlength - 4) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	len = in[p] + 256 * in[p + 1];
	p += 2;
	nlen = in[p] + 256 * in[p + 1];
	p += 2;

	// Check if 16-bit nlen is really the one's complement of len
	if (len + nlen != 65535) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	if ((*pos) + len >= outsize) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	// Read the literal data: len bytes are now stored in the out buffer
	if (p + len > inlength) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	for (n = 0; n < len; n++) {
		out[(*pos)++] = in[p++];
	}

	(*bp) = p * 8;
}

// Inflate the deflated data (cfr. deflate spec)
static enum png_error uz_inflate_data(struct png *png, u8 *out, u32 outsize, const u8 *in,
				      u32 insize, u32 inpos)
{
	u32 bp = 0; // Bit pointer in the in buffer
	u32 pos = 0; // Byte position in the out buffer

	u32 done = 0;

	while (done == 0) {
		u32 btype;

		// Ensure next bit doesn't point past the end of the buffer
		if ((bp >> 3) >= insize) {
			SET_ERROR(png, PNG_EMALFORMED);
			return png->error;
		}

		// Read block control bits
		done = read_bit(&bp, &in[inpos]);
		btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

		// Process control type appropriately
		if (btype == 3) {
			SET_ERROR(png, PNG_EMALFORMED);
			return png->error;
		} else if (btype == 0) {
			// No compression
			inflate_uncompressed(png, out, outsize, &in[inpos], &bp, &pos, insize);
		} else {
			// Compression, btype 01 or 10
			inflate_huffman(png, out, outsize, &in[inpos], &bp, &pos, insize, btype);
		}

		// Stop if an error has occured
		if (png->error != PNG_EOK) {
			return png->error;
		}
	}

	return png->error;
}

static enum png_error uz_inflate(struct png *png, u8 *out, u32 outsize, const u8 *in, u32 insize)
{
	// Two bytes for the zlib data header
	if (insize < 2) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
	if ((in[0] * 256 + in[1]) % 31 != 0) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// Error: Only compression method 8: Inflate with sliding window of 32k is supported by the PNG spec
	if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// The specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
	if (((in[1] >> 5) & 1) != 0) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// Create output buffer
	uz_inflate_data(png, out, outsize, in, insize, 2);

	return png->error;
}

// Paeth predictor, used by PNG filter type 4
static int paeth_predictor(int a, int b, int c)
{
	int p = a + b - c;
	int pa = p > a ? p - a : a - p;
	int pb = p > b ? p - b : b - p;
	int pc = p > c ? p - c : c - p;

	if (pa <= pb && pa <= pc)
		return a;
	else if (pb <= pc)
		return b;
	else
		return c;
}

static void unfilter_scanline(struct png *png, u8 *recon, const u8 *scanline, const u8 *precon,
			      u32 bytewidth, u8 filterType, u32 length)
{
	/*
	   For PNG filter method 0
	   Unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1)
	   precon is the previous unfiltered scanline, recon the result, scanline the current one
	   The incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
	   recon and scanline MAY be the same memory address! precon must be disjoint.
	*/

	u32 i;
	switch (filterType) {
	case 0:
		for (i = 0; i < length; i++)
			recon[i] = scanline[i];
		break;
	case 1:
		for (i = 0; i < bytewidth; i++)
			recon[i] = scanline[i];
		for (i = bytewidth; i < length; i++)
			recon[i] = scanline[i] + recon[i - bytewidth];
		break;
	case 2:
		if (precon)
			for (i = 0; i < length; i++)
				recon[i] = scanline[i] + precon[i];
		else
			for (i = 0; i < length; i++)
				recon[i] = scanline[i];
		break;
	case 3:
		if (precon) {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i] + precon[i] / 2;
			for (i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
		} else {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for (i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + recon[i - bytewidth] / 2;
		}
		break;
	case 4:
		if (precon) {
			for (i = 0; i < bytewidth; i++)
				recon[i] = (u8)(scanline[i] + paeth_predictor(0, precon[i], 0));
			for (i = bytewidth; i < length; i++)
				recon[i] = (u8)(scanline[i] +
						paeth_predictor(recon[i - bytewidth], precon[i],
								precon[i - bytewidth]));
		} else {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for (i = bytewidth; i < length; i++)
				recon[i] = (u8)(scanline[i] +
						paeth_predictor(recon[i - bytewidth], 0, 0));
		}
		break;
	default:
		SET_ERROR(png, PNG_EMALFORMED);
		break;
	}
}

static void unfilter(struct png *png, u8 *out, const u8 *in, u32 w, u32 h, u32 bpp)
{
	/*
	   For PNG filter method 0
	   This function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times)
	   out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
	   w and h are image dimensions or dimensions of reduced image, bpp is bpp per pixel
	   in and out are allowed to be the same memory address!
	*/

	u32 y;
	u8 *prevline = 0;

	// bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise
	u32 bytewidth = (bpp + 7) / 8;
	u32 linebytes = (w * bpp + 7) / 8;

	for (y = 0; y < h; y++) {
		u32 outindex = linebytes * y;
		u32 inindex = (1 + linebytes) * y;
		u8 filterType = in[inindex];

		unfilter_scanline(png, &out[outindex], &in[inindex + 1], prevline, bytewidth,
				  filterType, linebytes);
		if (png->error != PNG_EOK) {
			return;
		}

		prevline = &out[outindex];
	}
}

static void remove_padding_bits(u8 *out, const u8 *in, u32 olinebits, u32 ilinebits, u32 h)
{
	/*
	   After filtering there are still padding bpp if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers for the Adam7 code, the color convert code and the output to the user.
	   in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must have >= ilinebits*h bpp, out must have >= olinebits*h bpp, olinebits must be <= ilinebits
	   Also used to move bpp after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
	   Only useful if (ilinebits - olinebits) is a value in the range 1..7
	*/

	u32 y;
	u32 diff = ilinebits - olinebits;
	u32 obp = 0, ibp = 0; // Bit pointers
	for (y = 0; y < h; y++) {
		u32 x;
		for (x = 0; x < olinebits; x++) {
			u8 bit = (u8)((in[(ibp) >> 3] >> (7 - ((ibp)&0x7))) & 1);
			ibp++;

			if (bit == 0)
				out[(obp) >> 3] &= (u8)(~(1 << (7 - ((obp)&0x7))));
			else
				out[(obp) >> 3] |= (1 << (7 - ((obp)&0x7)));
			++obp;
		}
		ibp += diff;
	}
}

// out buffer must be big enough to contain full image, and it must contain the full decompressed data from the IDAT chunks
static void post_process_scanlines(struct png *png, u8 *out, u8 *in, const struct png *info_png)
{
	u32 bpp = png_get_bpp(info_png);
	u32 w = info_png->width;
	u32 h = info_png->height;

	if (bpp == 0) {
		SET_ERROR(png, PNG_EMALFORMED);
		return;
	}

	if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
		unfilter(png, in, in, w, h, bpp);
		if (png->error != PNG_EOK) {
			return;
		}
		remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
	} else {
		unfilter(png, out, in, w, h, bpp);
	}
}

static enum png_format determine_format(struct png *png)
{
	switch (png->color_type) {
	case PNG_LUM:
		switch (png->color_depth) {
		case 1:
			return PNG_LUMINANCE1;
		case 2:
			return PNG_LUMINANCE2;
		case 4:
			return PNG_LUMINANCE4;
		case 8:
			return PNG_LUMINANCE8;
		default:
			return PNG_BADFORMAT;
		}
	case PNG_RGB:
		switch (png->color_depth) {
		case 8:
			return PNG_RGB8;
		case 16:
			return PNG_RGB16;
		default:
			return PNG_BADFORMAT;
		}
	case PNG_LUMA:
		switch (png->color_depth) {
		case 1:
			return PNG_LUMINANCE_ALPHA1;
		case 2:
			return PNG_LUMINANCE_ALPHA2;
		case 4:
			return PNG_LUMINANCE_ALPHA4;
		case 8:
			return PNG_LUMINANCE_ALPHA8;
		default:
			return PNG_BADFORMAT;
		}
	case PNG_RGBA:
		switch (png->color_depth) {
		case 8:
			return PNG_RGBA8;
		case 16:
			return PNG_RGBA16;
		default:
			return PNG_BADFORMAT;
		}
	default:
		return PNG_BADFORMAT;
	}
}

static void png_free_source(struct png *png)
{
	if (png->source.owning != 0) {
		free((void *)png->source.buffer);
	}

	png->source.buffer = NULL;
	png->source.size = 0;
	png->source.owning = 0;
}

// Read the information from the header and store it in the png_info
enum png_error png_header(struct png *png)
{
	if (png->error != PNG_EOK) {
		return png->error;
	}

	if (png->state != PNG_NEW) {
		return png->error;
	}

	// Minimum length of a valid PNG file is 29 bytes
	// FIXME: Verify this against the specification, or against the actual code below
	if (png->source.size < 29) {
		SET_ERROR(png, PNG_ENOTPNG);
		return png->error;
	}

	// Check that PNG header matches expected value
	if (png->source.buffer[0] != 137 || png->source.buffer[1] != 80 ||
	    png->source.buffer[2] != 78 || png->source.buffer[3] != 71 ||
	    png->source.buffer[4] != 13 || png->source.buffer[5] != 10 ||
	    png->source.buffer[6] != 26 || png->source.buffer[7] != 10) {
		SET_ERROR(png, PNG_ENOTPNG);
		return png->error;
	}

	// Check that the first chunk is the IHDR chunk
	if (MAKE_DWORD_PTR(png->source.buffer + 12) != CHUNK_IHDR) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// Read the values given in the header
	png->width = MAKE_DWORD_PTR(png->source.buffer + 16);
	png->height = MAKE_DWORD_PTR(png->source.buffer + 20);
	png->color_depth = png->source.buffer[24];
	png->color_type = (enum png_color)png->source.buffer[25];

	// Determine the color format
	png->format = determine_format(png);
	if (png->format == PNG_BADFORMAT) {
		SET_ERROR(png, PNG_EUNFORMAT);
		return png->error;
	}

	// Check that the compression method (byte 27) is 0 (only allowed value in spec)
	if (png->source.buffer[26] != 0) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// Check that the compression method (byte 27) is 0 (only allowed value in spec)
	if (png->source.buffer[27] != 0) {
		SET_ERROR(png, PNG_EMALFORMED);
		return png->error;
	}

	// Check that the compression method (byte 27) is 0 (spec allows 1, but png does not support it)
	if (png->source.buffer[28] != 0) {
		SET_ERROR(png, PNG_EUNINTERLACED);
		return png->error;
	}

	png->state = PNG_HEADER;
	return png->error;
}

// Read a PNG, the result will be in the same color type as the PNG (hence "generic")
enum png_error png_decode(struct png *png)
{
	const u8 *chunk;
	u8 *compressed;
	u8 *inflated;
	u32 compressed_size = 0, compressed_index = 0;
	u32 inflated_size;
	enum png_error error;

	if (png->error != PNG_EOK) {
		return png->error;
	}

	// Parse the main header, if necessary
	png_header(png);
	if (png->error != PNG_EOK) {
		return png->error;
	}

	// Not a PNG header -> crash
	if (png->state != PNG_HEADER) {
		return png->error;
	}

	// Release old result, if any
	if (png->buffer != 0) {
		free(png->buffer);
		png->buffer = 0;
		png->size = 0;
	}

	// First byte of the first chunk after the header
	chunk = png->source.buffer + 33;

	// Scan through the chunks, finding the size of all IDAT chunks, and also verify general compliance
	while (chunk < png->source.buffer + png->source.size) {
		u32 length;

		// Make sure chunk header is not larger than the total compressed
		if ((u32)(chunk - png->source.buffer + 12) > png->source.size) {
			SET_ERROR(png, PNG_EMALFORMED);
			return png->error;
		}

		// Get length; sanity check it
		length = png_chunk_length(chunk);
		if (length > S32_MAX) {
			SET_ERROR(png, PNG_EMALFORMED);
			return png->error;
		}

		// Make sure chunk header+paylaod is not larger than the total compressed
		if ((u32)(chunk - png->source.buffer + length + 12) > png->source.size) {
			SET_ERROR(png, PNG_EMALFORMED);
			return png->error;
		}

		// Parse chunks
		if (png_chunk_type(chunk) == CHUNK_IDAT) {
			compressed_size += length;
		} else if (png_chunk_type(chunk) == CHUNK_IEND) {
			break;
		} else if (png_chunk_critical(chunk)) {
			SET_ERROR(png, PNG_EUNSUPPORTED);
			return png->error;
		}

		chunk += png_chunk_length(chunk) + 12;
	}

	// Allocate enough space for the (compressed and filtered) image data
	compressed = (u8 *)malloc(compressed_size);
	if (compressed == NULL) {
		SET_ERROR(png, PNG_ENOMEM);
		return png->error;
	}

	// Scan through the chunks again, this time copying the values into the compressed buffer.There's no reason to validate anything a second time
	chunk = png->source.buffer + 33;
	while (chunk < png->source.buffer + png->source.size) {
		u32 length;
		const u8 *data; // The data in the chunk

		length = png_chunk_length(chunk);
		data = chunk + 8;

		// Parse chunks
		if (png_chunk_type(chunk) == CHUNK_IDAT) {
			memcpy(compressed + compressed_index, data, length);
			compressed_index += length;
		} else if (png_chunk_type(chunk) == CHUNK_IEND) {
			break;
		}

		chunk += png_chunk_length(chunk) + 12;
	}

	// Allocate space to store inflated (but still filtered) data
	inflated_size = ((png->width * (png->height * png_get_bpp(png) + 7)) / 8) + png->height;
	inflated = (u8 *)malloc(inflated_size);
	if (inflated == NULL) {
		free(compressed);
		SET_ERROR(png, PNG_ENOMEM);
		return png->error;
	}

	// Decompress image data
	error = uz_inflate(png, inflated, inflated_size, compressed, compressed_size);
	if (error != PNG_EOK) {
		free(compressed);
		free(inflated);
		return png->error;
	}

	// Free the compressed compressed data
	free(compressed);

	// Allocate final image buffer
	png->size = (png->height * png->width * png_get_bpp(png) + 7) / 8;
	png->buffer = (u8 *)malloc(png->size);
	if (png->buffer == NULL) {
		free(inflated);
		png->size = 0;
		SET_ERROR(png, PNG_ENOMEM);
		return png->error;
	}

	// Unfilter scanlines
	post_process_scanlines(png, png->buffer, inflated, png);
	free(inflated);

	if (png->error != PNG_EOK) {
		free(png->buffer);
		png->buffer = NULL;
		png->size = 0;
	} else {
		png->state = PNG_DECODED;
	}

	// Done!
	png_free_source(png);

	return png->error;
}

static struct png *png_new(void)
{
	struct png *png;

	png = (struct png *)malloc(sizeof(struct png));
	if (png == NULL) {
		return NULL;
	}

	png->buffer = NULL;
	png->size = 0;

	png->width = png->height = 0;

	png->color_type = PNG_RGBA;
	png->color_depth = 8;
	png->format = PNG_RGBA8;

	png->state = PNG_NEW;

	png->error = PNG_EOK;
	png->error_line = 0;

	png->source.buffer = NULL;
	png->source.size = 0;
	png->source.owning = 0;

	return png;
}

void png_free(struct png *png)
{
	// Deallocate image buffer
	/* if (png->buffer != NULL) { */
	/* 	free(png->buffer); */
	/* } */

	// Deallocate source buffer, if necessary
	png_free_source(png);

	// Deallocate struct itself
	free(png);
}

u32 png_get_bpp(const struct png *png)
{
	int depth = 0;
	switch (png->color_type) {
	case PNG_LUM:
		depth = 1;
		break;
	case PNG_RGB:
		depth = 3;
		break;
	case PNG_LUMA:
		depth = 2;
		break;
	case PNG_RGBA:
		depth = 4;
		break;
	default:
		depth = 0;
		break;
	}

	return png->color_depth * depth;
}

struct bmp *png_load(const char *path)
{
	struct png *png = png_new();
	if (!png)
		return NULL;

	void *buf = read(path);
	if (!png) {
		SET_ERROR(png, PNG_ENOTFOUND);
		png_free(png);
		return NULL;
	}

	png->source.buffer = buf;
	png->source.size = stat(path);
	png->source.owning = 1;

	png_decode(png);
	assert(png->error == PNG_EOK);

	struct bmp *bmp = malloc(sizeof(*bmp));
	bmp->width = png->width;
	bmp->height = png->height;
	bmp->data = png->buffer;
	bmp->bpp = png_get_bpp(png);
	bmp->pitch = png->width * (bmp->bpp >> 3);

	png_free(png);

	return bmp;
}
