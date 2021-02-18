/*
LodePNG version 20201017

Copyright (c) 2005-2020 Lode Vandevenne
Copyright (c) 2021 Marvin Borner

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

#ifndef PNG_H
#define PNG_H

#include <def.h>
extern const char *PNG_VERSION_STRING;

/*
The following #defines are used to create code sections. They can be disabled
to disable code sections, which can give faster compile time and smaller binary.
The "NO_COMPILE" defines are designed to be used to pass as defines to the
compiler command to disable them without modifying this header, e.g.
-DPNG_NO_COMPILE_ZLIB for gcc.
In addition to those below, you can also define PNG_NO_COMPILE_CRC to
allow implementing a custom png_crc32.
*/
/*deflate & zlib. If disabled, you must specify alternative zlib functions in
the custom_zlib field of the compress and decompress settings*/
#ifndef PNG_NO_COMPILE_ZLIB
#define PNG_COMPILE_ZLIB
#endif

/*png encoder and png decoder*/
#ifndef PNG_NO_COMPILE_PNG
#define PNG_COMPILE_PNG
#endif

/*deflate&zlib decoder and png decoder*/
#ifndef PNG_NO_COMPILE_DECODER
#define PNG_COMPILE_DECODER
#endif

/*deflate&zlib encoder and png encoder*/
#ifndef PNG_NO_COMPILE_ENCODER
#define PNG_COMPILE_ENCODER
#endif

/*the optional built in harddisk file loading and saving functions*/
#ifndef PNG_NO_COMPILE_DISK
#define PNG_COMPILE_DISK
#endif

/*support for chunks other than IHDR, IDAT, PLTE, tRNS, IEND: ancillary and unknown chunks*/
#ifndef PNG_NO_COMPILE_ANCILLARY_CHUNKS
#define PNG_COMPILE_ANCILLARY_CHUNKS
#endif

/*ability to convert error numerical codes to English text string*/
#ifndef PNG_NO_COMPILE_ERROR_TEXT
#define PNG_COMPILE_ERROR_TEXT
#endif

/*Compile the default allocators (C's free, malloc and realloc). If you disable this,
you can define the functions png_free, png_malloc and png_realloc in your
source files with custom allocators.*/
#ifndef PNG_NO_COMPILE_ALLOCATORS
#define PNG_COMPILE_ALLOCATORS
#endif

/*compile the C++ version (you can disable the C++ wrapper here even when compiling for C++)*/
#ifdef __cplusplus
#ifndef PNG_NO_COMPILE_CPP
#define PNG_COMPILE_CPP
#endif
#endif

#ifdef PNG_COMPILE_CPP
#include <string>
#include <vector>
#endif /*PNG_COMPILE_CPP*/

#ifdef PNG_COMPILE_PNG
/*The PNG color types (also used for raw image).*/
typedef enum pngColorType {
	LCT_GREY = 0, /*grayscale: 1,2,4,8,16 bit*/
	LCT_RGB = 2, /*RGB: 8,16 bit*/
	LCT_PALETTE = 3, /*palette: 1,2,4,8 bit*/
	LCT_GREY_ALPHA = 4, /*grayscale with alpha: 8,16 bit*/
	LCT_RGBA = 6, /*RGB with alpha: 8,16 bit*/
	/*LCT_MAX_OCTET_VALUE lets the compiler allow this enum to represent any invalid
  byte value from 0 to 255 that could be present in an invalid PNG file header. Do
  not use, compare with or set the name LCT_MAX_OCTET_VALUE, instead either use
  the valid color type names above, or numeric values like 1 or 7 when checking for
  particular disallowed color type byte values, or cast to integer to print it.*/
	LCT_MAX_OCTET_VALUE = 255
} pngColorType;

#ifdef PNG_COMPILE_DECODER
/*
Converts PNG data in memory to raw pixel data.
out: Output parameter. Pointer to buffer that will contain the raw pixel data.
     After decoding, its size is w * h * (bytes per pixel) bytes larger than
     initially. Bytes per pixel depends on colortype and bitdepth.
     Must be freed after usage with free(*out).
     Note: for 16-bit per channel colors, uses big endian format like PNG does.
w: Output parameter. Pointer to width of pixel data.
h: Output parameter. Pointer to height of pixel data.
in: Memory buffer with the PNG file.
insize: size of the in buffer.
colortype: the desired color type for the raw output image. See explanation on PNG color types.
bitdepth: the desired bit depth for the raw output image. See explanation on PNG color types.
Return value: png error code (0 means no error).
*/
u32 png_decode_memory(u8 **out, u32 *w, u32 *h, const u8 *in, u32 insize, pngColorType colortype,
		      u32 bitdepth);

/*Same as png_decode_memory, but always decodes to 32-bit RGBA raw image*/
u32 png_decode32(u8 **out, u32 *w, u32 *h, const u8 *in, u32 insize);

/*Same as png_decode_memory, but always decodes to 24-bit RGB raw image*/
u32 png_decode24(u8 **out, u32 *w, u32 *h, const u8 *in, u32 insize);

#ifdef PNG_COMPILE_DISK
/*
Load PNG from disk, from file with given name.
Same as the other decode functions, but instead takes a filename as input.
*/
u32 png_decode_file(u8 **out, u32 *w, u32 *h, const char *filename, pngColorType colortype,
		    u32 bitdepth);

/*Same as png_decode_file, but always decodes to 32-bit RGBA raw image.*/
u32 png_decode32_file(u8 **out, u32 *w, u32 *h, const char *filename);

/*Same as png_decode_file, but always decodes to 24-bit RGB raw image.*/
u32 png_decode24_file(u8 **out, u32 *w, u32 *h, const char *filename);
#endif /*PNG_COMPILE_DISK*/
#endif /*PNG_COMPILE_DECODER*/

#ifdef PNG_COMPILE_ENCODER
/*
Converts raw pixel data into a PNG image in memory. The colortype and bitdepth
  of the output PNG image cannot be chosen, they are automatically determined
  by the colortype, bitdepth and content of the input pixel data.
  Note: for 16-bit per channel colors, needs big endian format like PNG does.
out: Output parameter. Pointer to buffer that will contain the PNG image data.
     Must be freed after usage with free(*out).
outsize: Output parameter. Pointer to the size in bytes of the out buffer.
image: The raw pixel data to encode. The size of this buffer should be
       w * h * (bytes per pixel), bytes per pixel depends on colortype and bitdepth.
w: width of the raw pixel data in pixels.
h: height of the raw pixel data in pixels.
colortype: the color type of the raw input image. See explanation on PNG color types.
bitdepth: the bit depth of the raw input image. See explanation on PNG color types.
Return value: png error code (0 means no error).
*/
u32 png_encode_memory(u8 **out, u32 *outsize, const u8 *image, u32 w, u32 h, pngColorType colortype,
		      u32 bitdepth);

/*Same as png_encode_memory, but always encodes from 32-bit RGBA raw image.*/
u32 png_encode32(u8 **out, u32 *outsize, const u8 *image, u32 w, u32 h);

/*Same as png_encode_memory, but always encodes from 24-bit RGB raw image.*/
u32 png_encode24(u8 **out, u32 *outsize, const u8 *image, u32 w, u32 h);

#ifdef PNG_COMPILE_DISK
/*
Converts raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
u32 png_encode_file(const char *filename, const u8 *image, u32 w, u32 h, pngColorType colortype,
		    u32 bitdepth);

/*Same as png_encode_file, but always encodes from 32-bit RGBA raw image.*/
u32 png_encode32_file(const char *filename, const u8 *image, u32 w, u32 h);

/*Same as png_encode_file, but always encodes from 24-bit RGB raw image.*/
u32 png_encode24_file(const char *filename, const u8 *image, u32 w, u32 h);
#endif /*PNG_COMPILE_DISK*/
#endif /*PNG_COMPILE_ENCODER*/

#ifdef PNG_COMPILE_CPP
namespace png
{
#ifdef PNG_COMPILE_DECODER
/*Same as png_decode_memory, but decodes to an std::vector. The colortype
is the format to output the pixels to. Default is RGBA 8-bit per channel.*/
u32 decode(std::vector<u8> &out, u32 &w, u32 &h, const u8 *in, u32 insize,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
u32 decode(std::vector<u8> &out, u32 &w, u32 &h, const std::vector<u8> &in,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
#ifdef PNG_COMPILE_DISK
/*
Converts PNG file from disk to raw pixel data in memory.
Same as the other decode functions, but instead takes a filename as input.
*/
u32 decode(std::vector<u8> &out, u32 &w, u32 &h, const std::string &filename,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
#endif /* PNG_COMPILE_DISK */
#endif /* PNG_COMPILE_DECODER */

#ifdef PNG_COMPILE_ENCODER
/*Same as png_encode_memory, but encodes to an std::vector. colortype
is that of the raw input data. The output PNG color type will be auto chosen.*/
u32 encode(std::vector<u8> &out, const u8 *in, u32 w, u32 h, pngColorType colortype = LCT_RGBA,
	   u32 bitdepth = 8);
u32 encode(std::vector<u8> &out, const std::vector<u8> &in, u32 w, u32 h,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
#ifdef PNG_COMPILE_DISK
/*
Converts 32-bit RGBA raw pixel data into a PNG file on disk.
Same as the other encode functions, but instead takes a filename as output.
NOTE: This overwrites existing files without warning!
*/
u32 encode(const std::string &filename, const u8 *in, u32 w, u32 h,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
u32 encode(const std::string &filename, const std::vector<u8> &in, u32 w, u32 h,
	   pngColorType colortype = LCT_RGBA, u32 bitdepth = 8);
#endif /* PNG_COMPILE_DISK */
#endif /* PNG_COMPILE_ENCODER */
} /* namespace png */
#endif /*PNG_COMPILE_CPP*/
#endif /*PNG_COMPILE_PNG*/

#ifdef PNG_COMPILE_ERROR_TEXT
/*Returns an English description of the numerical error code.*/
const char *png_error_text(u32 code);
#endif /*PNG_COMPILE_ERROR_TEXT*/

#ifdef PNG_COMPILE_DECODER
/*Settings for zlib decompression*/
typedef struct pngDecompressSettings pngDecompressSettings;
struct pngDecompressSettings {
	/* Check pngDecoderSettings for more ignorable errors such as ignore_crc */
	u32 ignore_adler32; /*if 1, continue and don't give an error message if the Adler32 checksum is corrupted*/
	u32 ignore_nlen; /*ignore complement of len checksum in uncompressed blocks*/

	/*Maximum decompressed size, beyond this the decoder may (and is encouraged to) stop decoding,
  return an error, output a data size > max_output_size and all the data up to that point. This is
  not hard limit nor a guarantee, but can prevent excessive memory usage. This setting is
  ignored by the PNG decoder, but is used by the deflate/zlib decoder and can be used by custom ones.
  Set to 0 to impose no limit (the default).*/
	u32 max_output_size;

	/*use custom zlib decoder instead of built in one (default: null).
  Should return 0 if success, any non-0 if error (numeric value not exposed).*/
	u32 (*custom_zlib)(u8 **, u32 *, const u8 *, u32, const pngDecompressSettings *);
	/*use custom deflate decoder instead of built in one (default: null)
  if custom_zlib is not null, custom_inflate is ignored (the zlib format uses deflate).
  Should return 0 if success, any non-0 if error (numeric value not exposed).*/
	u32 (*custom_inflate)(u8 **, u32 *, const u8 *, u32, const pngDecompressSettings *);

	const void *custom_context; /*optional custom settings for custom functions*/
};

extern const pngDecompressSettings png_default_decompress_settings;
void png_decompress_settings_init(pngDecompressSettings *settings);
#endif /*PNG_COMPILE_DECODER*/

#ifdef PNG_COMPILE_ENCODER
/*
Settings for zlib compression. Tweaking these settings tweaks the balance
between speed and compression ratio.
*/
typedef struct pngCompressSettings pngCompressSettings;
struct pngCompressSettings /*deflate = compress*/ {
	/*LZ77 related settings*/
	u32 btype; /*the block type for LZ (0, 1, 2 or 3, see zlib standard). Should be 2 for proper compression.*/
	u32 use_lz77; /*whether or not to use LZ77. Should be 1 for proper compression.*/
	u32 windowsize; /*must be a power of two <= 32768. higher compresses more but is slower. Default value: 2048.*/
	u32 minmatch; /*minimum lz77 length. 3 is normally best, 6 can be better for some PNGs. Default: 0*/
	u32 nicematch; /*stop searching if >= this length found. Set to 258 for best compression. Default: 128*/
	u32 lazymatching; /*use lazy matching: better compression but a bit slower. Default: true*/

	/*use custom zlib encoder instead of built in one (default: null)*/
	u32 (*custom_zlib)(u8 **, u32 *, const u8 *, u32, const pngCompressSettings *);
	/*use custom deflate encoder instead of built in one (default: null)
  if custom_zlib is used, custom_deflate is ignored since only the built in
  zlib function will call custom_deflate*/
	u32 (*custom_deflate)(u8 **, u32 *, const u8 *, u32, const pngCompressSettings *);

	const void *custom_context; /*optional custom settings for custom functions*/
};

extern const pngCompressSettings png_default_compress_settings;
void png_compress_settings_init(pngCompressSettings *settings);
#endif /*PNG_COMPILE_ENCODER*/

#ifdef PNG_COMPILE_PNG
/*
Color mode of an image. Contains all information required to decode the pixel
bits to RGBA colors. This information is the same as used in the PNG file
format, and is used both for PNG and raw image data in png.
*/
typedef struct pngColorMode {
	/*header (IHDR)*/
	pngColorType
		colortype; /*color type, see PNG standard or documentation further in this header file*/
	u32 bitdepth; /*bits per sample, see PNG standard or documentation further in this header file*/

	/*
  palette (PLTE and tRNS)

  Dynamically allocated with the colors of the palette, including alpha.
  This field may not be allocated directly, use png_color_mode_init first,
  then png_palette_add per color to correctly initialize it (to ensure size
  of exactly 1024 bytes).

  The alpha channels must be set as well, set them to 255 for opaque images.

  When decoding, by default you can ignore this palette, since png already
  fills the palette colors in the pixels of the raw RGBA output.

  The palette is only supported for color type 3.
  */
	u8 *palette; /*palette in RGBARGBA... order. Must be either 0, or when allocated must have 1024 bytes*/
	u32 palettesize; /*palette size in number of colors (amount of used bytes is 4 * palettesize)*/

	/*
  transparent color key (tRNS)

  This color uses the same bit depth as the bitdepth value in this struct, which can be 1-bit to 16-bit.
  For grayscale PNGs, r, g and b will all 3 be set to the same.

  When decoding, by default you can ignore this information, since png sets
  pixels with this key to transparent already in the raw RGBA output.

  The color key is only supported for color types 0 and 2.
  */
	u32 key_defined; /*is a transparent color key given? 0 = false, 1 = true*/
	u32 key_r; /*red/grayscale component of color key*/
	u32 key_g; /*green component of color key*/
	u32 key_b; /*blue component of color key*/
} pngColorMode;

/*init, cleanup and copy functions to use with this struct*/
void png_color_mode_init(pngColorMode *info);
void png_color_mode_cleanup(pngColorMode *info);
/*return value is error code (0 means no error)*/
u32 png_color_mode_copy(pngColorMode *dest, const pngColorMode *source);
/* Makes a temporary pngColorMode that does not need cleanup (no palette) */
pngColorMode png_color_mode_make(pngColorType colortype, u32 bitdepth);

void png_palette_clear(pngColorMode *info);
/*add 1 color to the palette*/
u32 png_palette_add(pngColorMode *info, u8 r, u8 g, u8 b, u8 a);

/*get the total amount of bits per pixel, based on colortype and bitdepth in the struct*/
u32 png_get_bpp(const pngColorMode *info);
/*get the amount of color channels used, based on colortype in the struct.
If a palette is used, it counts as 1 channel.*/
u32 png_get_channels(const pngColorMode *info);
/*is it a grayscale type? (only colortype 0 or 4)*/
u32 png_is_greyscale_type(const pngColorMode *info);
/*has it got an alpha channel? (only colortype 2 or 6)*/
u32 png_is_alpha_type(const pngColorMode *info);
/*has it got a palette? (only colortype 3)*/
u32 png_is_palette_type(const pngColorMode *info);
/*only returns true if there is a palette and there is a value in the palette with alpha < 255.
Loops through the palette to check this.*/
u32 png_has_palette_alpha(const pngColorMode *info);
/*
Check if the given color info indicates the possibility of having non-opaque pixels in the PNG image.
Returns true if the image can have translucent or invisible pixels (it still be opaque if it doesn't use such pixels).
Returns false if the image can only have opaque pixels.
In detail, it returns true only if it's a color type with alpha, or has a palette with non-opaque values,
or if "key_defined" is true.
*/
u32 png_can_have_alpha(const pngColorMode *info);
/*Returns the byte size of a raw image buffer with given width, height and color mode*/
u32 png_get_raw_size(u32 w, u32 h, const pngColorMode *color);

#ifdef PNG_COMPILE_ANCILLARY_CHUNKS
/*The information of a Time chunk in PNG.*/
typedef struct pngTime {
	u32 year; /*2 bytes used (0-65535)*/
	u32 month; /*1-12*/
	u32 day; /*1-31*/
	u32 hour; /*0-23*/
	u32 minute; /*0-59*/
	u32 second; /*0-60 (to allow for leap seconds)*/
} pngTime;
#endif /*PNG_COMPILE_ANCILLARY_CHUNKS*/

/*Information about the PNG image, except pixels, width and height.*/
typedef struct pngInfo {
	/*header (IHDR), palette (PLTE) and transparency (tRNS) chunks*/
	u32 compression_method; /*compression method of the original file. Always 0.*/
	u32 filter_method; /*filter method of the original file*/
	u32 interlace_method; /*interlace method of the original file: 0=none, 1=Adam7*/
	pngColorMode color; /*color type and bits, palette and transparency of the PNG file*/

#ifdef PNG_COMPILE_ANCILLARY_CHUNKS
	/*
  Suggested background color chunk (bKGD)

  This uses the same color mode and bit depth as the PNG (except no alpha channel),
  with values truncated to the bit depth in the u32 integer.

  For grayscale and palette PNGs, the value is stored in background_r. The values
  in background_g and background_b are then unused.

  So when decoding, you may get these in a different color mode than the one you requested
  for the raw pixels.

  When encoding with auto_convert, you must use the color model defined in info_png.color for
  these values. The encoder normally ignores info_png.color when auto_convert is on, but will
  use it to interpret these values (and convert copies of them to its chosen color model).

  When encoding, avoid setting this to an expensive color, such as a non-gray value
  when the image is gray, or the compression will be worse since it will be forced to
  write the PNG with a more expensive color mode (when auto_convert is on).

  The decoder does not use this background color to edit the color of pixels. This is a
  completely optional metadata feature.
  */
	u32 background_defined; /*is a suggested background color given?*/
	u32 background_r; /*red/gray/palette component of suggested background color*/
	u32 background_g; /*green component of suggested background color*/
	u32 background_b; /*blue component of suggested background color*/

	/*
  Non-international text chunks (tEXt and zTXt)

  The char** arrays each contain num strings. The actual messages are in
  text_strings, while text_keys are keywords that give a short description what
  the actual text represents, e.g. Title, Author, Description, or anything else.

  All the string fields below including strings, keys, names and language tags are null terminated.
  The PNG specification uses null characters for the keys, names and tags, and forbids null
  characters to appear in the main text which is why we can use null termination everywhere here.

  A keyword is minimum 1 character and maximum 79 characters long (plus the
  additional null terminator). It's discouraged to use a single line length
  longer than 79 characters for texts.

  Don't allocate these text buffers yourself. Use the init/cleanup functions
  correctly and use png_add_text and png_clear_text.

  Standard text chunk keywords and strings are encoded using Latin-1.
  */
	u32 text_num; /*the amount of texts in these char** buffers (there may be more texts in itext)*/
	char **text_keys; /*the keyword of a text chunk (e.g. "Comment")*/
	char **text_strings; /*the actual text*/

	/*
  International text chunks (iTXt)
  Similar to the non-international text chunks, but with additional strings
  "langtags" and "transkeys", and the following text encodings are used:
  keys: Latin-1, langtags: ASCII, transkeys and strings: UTF-8.
  keys must be 1-79 characters (plus the additional null terminator), the other
  strings are any length.
  */
	u32 itext_num; /*the amount of international texts in this PNG*/
	char **itext_keys; /*the English keyword of the text chunk (e.g. "Comment")*/
	char **itext_langtags; /*language tag for this text's language, ISO/IEC 646 string, e.g. ISO 639 language tag*/
	char **itext_transkeys; /*keyword translated to the international language - UTF-8 string*/
	char **itext_strings; /*the actual international text - UTF-8 string*/

	/*time chunk (tIME)*/
	u32 time_defined; /*set to 1 to make the encoder generate a tIME chunk*/
	pngTime time;

	/*phys chunk (pHYs)*/
	u32 phys_defined; /*if 0, there is no pHYs chunk and the values below are undefined, if 1 else there is one*/
	u32 phys_x; /*pixels per unit in x direction*/
	u32 phys_y; /*pixels per unit in y direction*/
	u32 phys_unit; /*may be 0 (unknown unit) or 1 (metre)*/

	/*
  Color profile related chunks: gAMA, cHRM, sRGB, iCPP

  png does not apply any color conversions on pixels in the encoder or decoder and does not interpret these color
  profile values. It merely passes on the information. If you wish to use color profiles and convert colors, please
  use these values with a color management library.

  See the PNG, ICC and sRGB specifications for more information about the meaning of these values.
  */

	/* gAMA chunk: optional, overridden by sRGB or iCCP if those are present. */
	u32 gama_defined; /* Whether a gAMA chunk is present (0 = not present, 1 = present). */
	u32 gama_gamma; /* Gamma exponent times 100000 */

	/* cHRM chunk: optional, overridden by sRGB or iCCP if those are present. */
	u32 chrm_defined; /* Whether a cHRM chunk is present (0 = not present, 1 = present). */
	u32 chrm_white_x; /* White Point x times 100000 */
	u32 chrm_white_y; /* White Point y times 100000 */
	u32 chrm_red_x; /* Red x times 100000 */
	u32 chrm_red_y; /* Red y times 100000 */
	u32 chrm_green_x; /* Green x times 100000 */
	u32 chrm_green_y; /* Green y times 100000 */
	u32 chrm_blue_x; /* Blue x times 100000 */
	u32 chrm_blue_y; /* Blue y times 100000 */

	/*
  sRGB chunk: optional. May not appear at the same time as iCCP.
  If gAMA is also present gAMA must contain value 45455.
  If cHRM is also present cHRM must contain respectively 31270,32900,64000,33000,30000,60000,15000,6000.
  */
	u32 srgb_defined; /* Whether an sRGB chunk is present (0 = not present, 1 = present). */
	u32 srgb_intent; /* Rendering intent: 0=perceptual, 1=rel. colorimetric, 2=saturation, 3=abs. colorimetric */

	/*
  iCCP chunk: optional. May not appear at the same time as sRGB.

  png does not parse or use the ICC profile (except its color space header field for an edge case), a
  separate library to handle the ICC data (not included in png) format is needed to use it for color
  management and conversions.

  For encoding, if iCCP is present, gAMA and cHRM are recommended to be added as well with values that match the ICC
  profile as closely as possible, if you wish to do this you should provide the correct values for gAMA and cHRM and
  enable their '_defined' flags since png will not automatically compute them from the ICC profile.

  For encoding, the ICC profile is required by the PNG specification to be an "RGB" profile for non-gray
  PNG color types and a "GRAY" profile for gray PNG color types. If you disable auto_convert, you must ensure
  the ICC profile type matches your requested color type, else the encoder gives an error. If auto_convert is
  enabled (the default), and the ICC profile is not a good match for the pixel data, this will result in an encoder
  error if the pixel data has non-gray pixels for a GRAY profile, or a silent less-optimal compression of the pixel
  data if the pixels could be encoded as grayscale but the ICC profile is RGB.

  To avoid this do not set an ICC profile in the image unless there is a good reason for it, and when doing so
  make sure you compute it carefully to avoid the above problems.
  */
	u32 iccp_defined; /* Whether an iCCP chunk is present (0 = not present, 1 = present). */
	char *iccp_name; /* Null terminated string with profile name, 1-79 bytes */
	/*
  The ICC profile in iccp_profile_size bytes.
  Don't allocate this buffer yourself. Use the init/cleanup functions
  correctly and use png_set_icc and png_clear_icc.
  */
	u8 *iccp_profile;
	u32 iccp_profile_size; /* The size of iccp_profile in bytes */

	/* End of color profile related chunks */

	/*
  unknown chunks: chunks not known by png, passed on byte for byte.

  There are 3 buffers, one for each position in the PNG where unknown chunks can appear.
  Each buffer contains all unknown chunks for that position consecutively.
  The 3 positions are:
  0: between IHDR and PLTE, 1: between PLTE and IDAT, 2: between IDAT and IEND.

  For encoding, do not store critical chunks or known chunks that are enabled with a "_defined" flag
  above in here, since the encoder will blindly follow this and could then encode an invalid PNG file
  (such as one with two IHDR chunks or the disallowed combination of sRGB with iCCP). But do use
  this if you wish to store an ancillary chunk that is not supported by png (such as sPLT or hIST),
  or any non-standard PNG chunk.

  Do not allocate or traverse this data yourself. Use the chunk traversing functions declared
  later, such as png_chunk_next and png_chunk_append, to read/write this struct.
  */
	u8 *unknown_chunks_data[3];
	u32 unknown_chunks_size[3]; /*size in bytes of the unknown chunks, given for protection*/
#endif /*PNG_COMPILE_ANCILLARY_CHUNKS*/
} pngInfo;

/*init, cleanup and copy functions to use with this struct*/
void png_info_init(pngInfo *info);
void png_info_cleanup(pngInfo *info);
/*return value is error code (0 means no error)*/
u32 png_info_copy(pngInfo *dest, const pngInfo *source);

#ifdef PNG_COMPILE_ANCILLARY_CHUNKS
u32 png_add_text(pngInfo *info, const char *key, const char *str); /*push back both texts at once*/
void png_clear_text(pngInfo *info); /*use this to clear the texts again after you filled them in*/

u32 png_add_itext(pngInfo *info, const char *key, const char *langtag, const char *transkey,
		  const char *str); /*push back the 4 texts of 1 chunk at once*/
void png_clear_itext(pngInfo *info); /*use this to clear the itexts again after you filled them in*/

/*replaces if exists*/
u32 png_set_icc(pngInfo *info, const char *name, const u8 *profile, u32 profile_size);
void png_clear_icc(pngInfo *info); /*use this to clear the texts again after you filled them in*/
#endif /*PNG_COMPILE_ANCILLARY_CHUNKS*/

/*
Converts raw buffer from one color type to another color type, based on
pngColorMode structs to describe the input and output color type.
See the reference manual at the end of this header file to see which color conversions are supported.
return value = png error code (0 if all went ok, an error if the conversion isn't supported)
The out buffer must have size (w * h * bpp + 7) / 8, where bpp is the bits per pixel
of the output color type (png_get_bpp).
For < 8 bpp images, there should not be padding bits at the end of scanlines.
For 16-bit per channel colors, uses big endian format like PNG does.
Return value is png error code
*/
u32 png_convert(u8 *out, const u8 *in, const pngColorMode *mode_out, const pngColorMode *mode_in,
		u32 w, u32 h);

#ifdef PNG_COMPILE_DECODER
/*
Settings for the decoder. This contains settings for the PNG and the Zlib
decoder, but not the Info settings from the Info structs.
*/
typedef struct pngDecoderSettings {
	pngDecompressSettings zlibsettings; /*in here is the setting to ignore Adler32 checksums*/

	/* Check pngDecompressSettings for more ignorable errors such as ignore_adler32 */
	u32 ignore_crc; /*ignore CRC checksums*/
	u32 ignore_critical; /*ignore unknown critical chunks*/
	u32 ignore_end; /*ignore issues at end of file if possible (missing IEND chunk, too large chunk, ...)*/
	/* TODO: make a system involving warnings with levels and a strict mode instead. Other potentially recoverable
     errors: srgb rendering intent value, size of content of ancillary chunks, more than 79 characters for some
     strings, placement/combination rules for ancillary chunks, crc of unknown chunks, allowed characters
     in string keys, etc... */

	u32 color_convert; /*whether to convert the PNG to the color type you want. Default: yes*/

#ifdef PNG_COMPILE_ANCILLARY_CHUNKS
	u32 read_text_chunks; /*if false but remember_unknown_chunks is true, they're stored in the unknown chunks*/

	/*store all bytes from unknown chunks in the pngInfo (off by default, useful for a png editor)*/
	u32 remember_unknown_chunks;

	/* maximum size for decompressed text chunks. If a text chunk's text is larger than this, an error is returned,
  unless reading text chunks is disabled or this limit is set higher or disabled. Set to 0 to allow any size.
  By default it is a value that prevents unreasonably large strings from hogging memory. */
	u32 max_text_size;

	/* maximum size for compressed ICC chunks. If the ICC profile is larger than this, an error will be returned. Set to
  0 to allow any size. By default this is a value that prevents ICC profiles that would be much larger than any
  legitimate profile could be to hog memory. */
	u32 max_icc_size;
#endif /*PNG_COMPILE_ANCILLARY_CHUNKS*/
} pngDecoderSettings;

void png_decoder_settings_init(pngDecoderSettings *settings);
#endif /*PNG_COMPILE_DECODER*/

#ifdef PNG_COMPILE_ENCODER
/*automatically use color type with less bits per pixel if losslessly possible. Default: AUTO*/
typedef enum pngFilterStrategy {
	/*every filter at zero*/
	LFS_ZERO = 0,
	/*every filter at 1, 2, 3 or 4 (paeth), unlike LFS_ZERO not a good choice, but for testing*/
	LFS_ONE = 1,
	LFS_TWO = 2,
	LFS_THREE = 3,
	LFS_FOUR = 4,
	/*Use filter that gives minimum sum, as described in the official PNG filter heuristic.*/
	LFS_MINSUM,
	/*Use the filter type that gives smallest Shannon entropy for this scanline. Depending
  on the image, this is better or worse than minsum.*/
	LFS_ENTROPY,
	/*
  Brute-force-search PNG filters by compressing each filter for each scanline.
  Experimental, very slow, and only rarely gives better compression than MINSUM.
  */
	LFS_BRUTE_FORCE,
	/*use predefined_filters buffer: you specify the filter type for each scanline*/
	LFS_PREDEFINED
} pngFilterStrategy;

/*Gives characteristics about the integer RGBA colors of the image (count, alpha channel usage, bit depth, ...),
which helps decide which color model to use for encoding.
Used internally by default if "auto_convert" is enabled. Public because it's useful for custom algorithms.*/
typedef struct pngColorStats {
	u32 colored; /*not grayscale*/
	u32 key; /*image is not opaque and color key is possible instead of full alpha*/
	u16 key_r; /*key values, always as 16-bit, in 8-bit case the byte is duplicated, e.g. 65535 means 255*/
	u16 key_g;
	u16 key_b;
	u32 alpha; /*image is not opaque and alpha channel or alpha palette required*/
	u32 numcolors; /*amount of colors, up to 257. Not valid if bits == 16 or allow_palette is disabled.*/
	u8 palette[1024]; /*Remembers up to the first 256 RGBA colors, in no particular order, only valid when numcolors is valid*/
	u32 bits; /*bits per channel (not for palette). 1,2 or 4 for grayscale only. 16 if 16-bit per channel required.*/
	u32 numpixels;

	/*user settings for computing/using the stats*/
	u32 allow_palette; /*default 1. if 0, disallow choosing palette colortype in auto_choose_color, and don't count numcolors*/
	u32 allow_greyscale; /*default 1. if 0, choose RGB or RGBA even if the image only has gray colors*/
} pngColorStats;

void png_color_stats_init(pngColorStats *stats);

/*Get a pngColorStats of the image. The stats must already have been inited.
Returns error code (e.g. alloc fail) or 0 if ok.*/
u32 png_compute_color_stats(pngColorStats *stats, const u8 *image, u32 w, u32 h,
			    const pngColorMode *mode_in);

/*Settings for the encoder.*/
typedef struct pngEncoderSettings {
	pngCompressSettings zlibsettings; /*settings for the zlib encoder, such as window size, ...*/

	u32 auto_convert; /*automatically choose output PNG color type. Default: true*/

	/*If true, follows the official PNG heuristic: if the PNG uses a palette or lower than
  8 bit depth, set all filters to zero. Otherwise use the filter_strategy. Note that to
  completely follow the official PNG heuristic, filter_palette_zero must be true and
  filter_strategy must be LFS_MINSUM*/
	u32 filter_palette_zero;
	/*Which filter strategy to use when not using zeroes due to filter_palette_zero.
  Set filter_palette_zero to 0 to ensure always using your chosen strategy. Default: LFS_MINSUM*/
	pngFilterStrategy filter_strategy;
	/*used if filter_strategy is LFS_PREDEFINED. In that case, this must point to a buffer with
  the same length as the amount of scanlines in the image, and each value must <= 5. You
  have to cleanup this buffer, png will never free it. Don't forget that filter_palette_zero
  must be set to 0 to ensure this is also used on palette or low bitdepth images.*/
	const u8 *predefined_filters;

	/*force creating a PLTE chunk if colortype is 2 or 6 (= a suggested palette).
  If colortype is 3, PLTE is _always_ created.*/
	u32 force_palette;
#ifdef PNG_COMPILE_ANCILLARY_CHUNKS
	/*add png identifier and version as a text chunk, for debugging*/
	u32 add_id;
	/*encode text chunks as zTXt chunks instead of tEXt chunks, and use compression in iTXt chunks*/
	u32 text_compression;
#endif /*PNG_COMPILE_ANCILLARY_CHUNKS*/
} pngEncoderSettings;

void png_encoder_settings_init(pngEncoderSettings *settings);
#endif /*PNG_COMPILE_ENCODER*/

#if defined(PNG_COMPILE_DECODER) || defined(PNG_COMPILE_ENCODER)
/*The settings, state and information for extended encoding and decoding.*/
typedef struct pngState {
#ifdef PNG_COMPILE_DECODER
	pngDecoderSettings decoder; /*the decoding settings*/
#endif /*PNG_COMPILE_DECODER*/
#ifdef PNG_COMPILE_ENCODER
	pngEncoderSettings encoder; /*the encoding settings*/
#endif /*PNG_COMPILE_ENCODER*/
	pngColorMode
		info_raw; /*specifies the format in which you would like to get the raw pixel buffer*/
	pngInfo info_png; /*info of the PNG image obtained after decoding*/
	u32 error;
} pngState;

/*init, cleanup and copy functions to use with this struct*/
void png_state_init(pngState *state);
void png_state_cleanup(pngState *state);
void png_state_copy(pngState *dest, const pngState *source);
#endif /* defined(PNG_COMPILE_DECODER) || defined(PNG_COMPILE_ENCODER) */

#ifdef PNG_COMPILE_DECODER
/*
Same as png_decode_memory, but uses a pngState to allow custom settings and
getting much more information about the PNG image and color mode.
*/
u32 png_decode(u8 **out, u32 *w, u32 *h, pngState *state, const u8 *in, u32 insize);

/*
Read the PNG header, but not the actual data. This returns only the information
that is in the IHDR chunk of the PNG, such as width, height and color type. The
information is placed in the info_png field of the pngState.
*/
u32 png_inspect(u32 *w, u32 *h, pngState *state, const u8 *in, u32 insize);
#endif /*PNG_COMPILE_DECODER*/

/*
Reads one metadata chunk (other than IHDR) of the PNG file and outputs what it
read in the state. Returns error code on failure.
Use png_inspect first with a new state, then e.g. png_chunk_find_const
to find the desired chunk type, and if non null use png_inspect_chunk (with
chunk_pointer - start_of_file as pos).
Supports most metadata chunks from the PNG standard (gAMA, bKGD, tEXt, ...).
Ignores unsupported, unknown, non-metadata or IHDR chunks (without error).
Requirements: &in[pos] must point to start of a chunk, must use regular
png_inspect first since format of most other chunks depends on IHDR, and if
there is a PLTE chunk, that one must be inspected before tRNS or bKGD.
*/
u32 png_inspect_chunk(pngState *state, u32 pos, const u8 *in, u32 insize);

#ifdef PNG_COMPILE_ENCODER
/*This function allocates the out buffer with standard malloc and stores the size in *outsize.*/
u32 png_encode(u8 **out, u32 *outsize, const u8 *image, u32 w, u32 h, pngState *state);
#endif /*PNG_COMPILE_ENCODER*/

/*
The png_chunk functions are normally not needed, except to traverse the
unknown chunks stored in the pngInfo struct, or add new ones to it.
It also allows traversing the chunks of an encoded PNG file yourself.

The chunk pointer always points to the beginning of the chunk itself, that is
the first byte of the 4 length bytes.

In the PNG file format, chunks have the following format:
-4 bytes length: length of the data of the chunk in bytes (chunk itself is 12 bytes longer)
-4 bytes chunk type (ASCII a-z,A-Z only, see below)
-length bytes of data (may be 0 bytes if length was 0)
-4 bytes of CRC, computed on chunk name + data

The first chunk starts at the 8th byte of the PNG file, the entire rest of the file
exists out of concatenated chunks with the above format.

PNG standard chunk ASCII naming conventions:
-First byte: uppercase = critical, lowercase = ancillary
-Second byte: uppercase = public, lowercase = private
-Third byte: must be uppercase
-Fourth byte: uppercase = unsafe to copy, lowercase = safe to copy
*/

/*
Gets the length of the data of the chunk. Total chunk length has 12 bytes more.
There must be at least 4 bytes to read from. If the result value is too large,
it may be corrupt data.
*/
u32 png_chunk_length(const u8 *chunk);

/*puts the 4-byte type in null terminated string*/
void png_chunk_type(char type[5], const u8 *chunk);

/*check if the type is the given type*/
u8 png_chunk_type_equals(const u8 *chunk, const char *type);

/*0: it's one of the critical chunk types, 1: it's an ancillary chunk (see PNG standard)*/
u8 png_chunk_ancillary(const u8 *chunk);

/*0: public, 1: private (see PNG standard)*/
u8 png_chunk_private(const u8 *chunk);

/*0: the chunk is unsafe to copy, 1: the chunk is safe to copy (see PNG standard)*/
u8 png_chunk_safetocopy(const u8 *chunk);

/*get pointer to the data of the chunk, where the input points to the header of the chunk*/
u8 *png_chunk_data(u8 *chunk);
const u8 *png_chunk_data_const(const u8 *chunk);

/*returns 0 if the crc is correct, 1 if it's incorrect (0 for OK as usual!)*/
u32 png_chunk_check_crc(const u8 *chunk);

/*generates the correct CRC from the data and puts it in the last 4 bytes of the chunk*/
void png_chunk_generate_crc(u8 *chunk);

/*
Iterate to next chunks, allows iterating through all chunks of the PNG file.
Input must be at the beginning of a chunk (result of a previous png_chunk_next call,
or the 8th byte of a PNG file which always has the first chunk), or alternatively may
point to the first byte of the PNG file (which is not a chunk but the magic header, the
function will then skip over it and return the first real chunk).
Will output pointer to the start of the next chunk, or at or beyond end of the file if there
is no more chunk after this or possibly if the chunk is corrupt.
Start this process at the 8th byte of the PNG file.
In a non-corrupt PNG file, the last chunk should have name "IEND".
*/
u8 *png_chunk_next(u8 *chunk, u8 *end);
const u8 *png_chunk_next_const(const u8 *chunk, const u8 *end);

/*Finds the first chunk with the given type in the range [chunk, end), or returns NULL if not found.*/
u8 *png_chunk_find(u8 *chunk, u8 *end, const char type[5]);
const u8 *png_chunk_find_const(const u8 *chunk, const u8 *end, const char type[5]);

/*
Appends chunk to the data in out. The given chunk should already have its chunk header.
The out variable and outsize are updated to reflect the new reallocated buffer.
Returns error code (0 if it went ok)
*/
u32 png_chunk_append(u8 **out, u32 *outsize, const u8 *chunk);

/*
Appends new chunk to out. The chunk to append is given by giving its length, type
and data separately. The type is a 4-letter string.
The out variable and outsize are updated to reflect the new reallocated buffer.
Returne error code (0 if it went ok)
*/
u32 png_chunk_create(u8 **out, u32 *outsize, u32 length, const char *type, const u8 *data);

/*Calculate CRC32 of buffer*/
u32 png_crc32(const u8 *buf, u32 len);
#endif /*PNG_COMPILE_PNG*/

#ifdef PNG_COMPILE_ZLIB
/*
This zlib part can be used independently to zlib compress and decompress a
buffer. It cannot be used to create gzip files however, and it only supports the
part of zlib that is required for PNG, it does not support dictionaries.
*/

#ifdef PNG_COMPILE_DECODER
/*Inflate a buffer. Inflate is the decompression step of deflate. Out buffer must be freed after use.*/
u32 png_inflate(u8 **out, u32 *outsize, const u8 *in, u32 insize,
		const pngDecompressSettings *settings);

/*
Decompresses Zlib data. Reallocates the out buffer and appends the data. The
data must be according to the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
u32 png_zlib_decompress(u8 **out, u32 *outsize, const u8 *in, u32 insize,
			const pngDecompressSettings *settings);
#endif /*PNG_COMPILE_DECODER*/

#ifdef PNG_COMPILE_ENCODER
/*
Compresses data with Zlib. Reallocates the out buffer and appends the data.
Zlib adds a small header and trailer around the deflate data.
The data is output in the format of the zlib specification.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid
buffer and *outsize its size in bytes. out must be freed by user after usage.
*/
u32 png_zlib_compress(u8 **out, u32 *outsize, const u8 *in, u32 insize,
		      const pngCompressSettings *settings);

/*
Find length-limited Huffman code for given frequencies. This function is in the
public interface only for tests, it's used internally by png_deflate.
*/
u32 png_huffman_code_lengths(u32 *lengths, const u32 *frequencies, u32 numcodes, u32 maxbitlen);

/*Compress a buffer with deflate. See RFC 1951. Out buffer must be freed after use.*/
u32 png_deflate(u8 **out, u32 *outsize, const u8 *in, u32 insize,
		const pngCompressSettings *settings);

#endif /*PNG_COMPILE_ENCODER*/
#endif /*PNG_COMPILE_ZLIB*/

#ifdef PNG_COMPILE_DISK
/*
Load a file from disk into buffer. The function allocates the out buffer, and
after usage you should free it.
out: output parameter, contains pointer to loaded buffer.
outsize: output parameter, size of the allocated out buffer
filename: the path to the file to load
return value: error code (0 means ok)
*/
u32 png_load_file(u8 **out, u32 *outsize, const char *filename);

/*
Save a file from buffer to disk. Warning, if it exists, this function overwrites
the file without warning!
buffer: the buffer to write
buffersize: size of the buffer to write
filename: the path to the file to save to
return value: error code (0 means ok)
*/
u32 png_save_file(const u8 *buffer, u32 buffersize, const char *filename);
#endif /*PNG_COMPILE_DISK*/

#ifdef PNG_COMPILE_CPP
/* The png C++ wrapper uses std::vectors instead of manually allocated memory buffers. */
namespace png
{
#ifdef PNG_COMPILE_PNG
class State : public pngState {
    public:
	State();
	State(const State &other);
	~State();
	State &operator=(const State &other);
};

#ifdef PNG_COMPILE_DECODER
/* Same as other png::decode, but using a State for more settings and information. */
u32 decode(std::vector<u8> &out, u32 &w, u32 &h, State &state, const u8 *in, u32 insize);
u32 decode(std::vector<u8> &out, u32 &w, u32 &h, State &state, const std::vector<u8> &in);
#endif /*PNG_COMPILE_DECODER*/

#ifdef PNG_COMPILE_ENCODER
/* Same as other png::encode, but using a State for more settings and information. */
u32 encode(std::vector<u8> &out, const u8 *in, u32 w, u32 h, State &state);
u32 encode(std::vector<u8> &out, const std::vector<u8> &in, u32 w, u32 h, State &state);
#endif /*PNG_COMPILE_ENCODER*/

#ifdef PNG_COMPILE_DISK
/*
Load a file from disk into an std::vector.
return value: error code (0 means ok)
*/
u32 load_file(std::vector<u8> &buffer, const std::string &filename);

/*
Save the binary data in an std::vector to a file on disk. The file is overwritten
without warning.
*/
u32 save_file(const std::vector<u8> &buffer, const std::string &filename);
#endif /* PNG_COMPILE_DISK */
#endif /* PNG_COMPILE_PNG */

#ifdef PNG_COMPILE_ZLIB
#ifdef PNG_COMPILE_DECODER
/* Zlib-decompress an u8 buffer */
u32 decompress(std::vector<u8> &out, const u8 *in, u32 insize,
	       const pngDecompressSettings &settings = png_default_decompress_settings);

/* Zlib-decompress an std::vector */
u32 decompress(std::vector<u8> &out, const std::vector<u8> &in,
	       const pngDecompressSettings &settings = png_default_decompress_settings);
#endif /* PNG_COMPILE_DECODER */

#ifdef PNG_COMPILE_ENCODER
/* Zlib-compress an u8 buffer */
u32 compress(std::vector<u8> &out, const u8 *in, u32 insize,
	     const pngCompressSettings &settings = png_default_compress_settings);

/* Zlib-compress an std::vector */
u32 compress(std::vector<u8> &out, const std::vector<u8> &in,
	     const pngCompressSettings &settings = png_default_compress_settings);
#endif /* PNG_COMPILE_ENCODER */
#endif /* PNG_COMPILE_ZLIB */
} /* namespace png */
#endif /*PNG_COMPILE_CPP*/

/*
TODO:
[.] test if there are no memory leaks or security exploits - done a lot but needs to be checked often
[.] check compatibility with various compilers  - done but needs to be redone for every newer version
[X] converting color to 16-bit per channel types
[X] support color profile chunk types (but never let them touch RGB values by default)
[ ] support all public PNG chunk types (almost done except sBIT, sPLT and hIST)
[ ] make sure encoder generates no chunks with size > (2^31)-1
[ ] partial decoding (stream processing)
[X] let the "isFullyOpaque" function check color keys and transparent palettes too
[X] better name for the variables "codes", "codesD", "codelengthcodes", "clcl" and "lldl"
[ ] allow treating some errors like warnings, when image is recoverable (e.g. 69, 57, 58)
[ ] make warnings like: oob palette, checksum fail, data after iend, wrong/unknown crit chunk, no null terminator in text, ...
[ ] error messages with line numbers (and version)
[ ] errors in state instead of as return code?
[ ] new errors/warnings like suspiciously big decompressed ztxt or iccp chunk
[ ] let the C++ wrapper catch exceptions coming from the standard library and return png error codes
[ ] allow user to provide custom color conversion functions, e.g. for premultiplied alpha, padding bits or not, ...
[ ] allow user to give data (void*) to custom allocator
[X] provide alternatives for C library functions not present on some platforms (memcpy, ...)
*/

#endif /*PNG_H inclusion guard*/
