#pragma once

/**
 * This file is whole based on the code from pugiutf.hpp 
 * (which is a part of pugixml parser)
 * Copyright (C) 2006-2012, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * http://pugixml.org/
 *
 * Pugixml library is distributed under the MIT License. See notice at the end
 * of this file.
 */

#include <stdint.h>
#include <assert.h>
#include <string>

// Character interface macros
#ifdef UTF_CONVERT_WCHAR_MODE
#	define UTF_CONVERT_TEXT(t) L ## t
#	define UTF_CONVERT_CHAR wchar_t
#else
#	define UTF_CONVERT_TEXT(t) t
#	define UTF_CONVERT_CHAR char
#endif

#define UTF_CONVERT_ALLOCATE malloc

namespace utf_convert
{
	// Character type used for all internal storage and operations; depends on UTF_CONVERT_WCHAR_MODE
	typedef UTF_CONVERT_CHAR char_t;

	// String type used for operations that work with STL string; depends on UTF_CONVERT_WCHAR_MODE
	typedef std::basic_string<UTF_CONVERT_CHAR, std::char_traits<UTF_CONVERT_CHAR>, std::allocator<UTF_CONVERT_CHAR> > string_t;

	struct opt_false
	{
		enum { value = 0 };
	};

	struct opt_true
	{
		enum { value = 1 };
	};
}

namespace utf_convert
{
	inline uint16_t endian_swap(uint16_t value)
	{
		return static_cast<uint16_t>(((value & 0xff) << 8) | (value >> 8));
	}

	inline uint32_t endian_swap(uint32_t value)
	{
		return ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value & 0xff0000) >> 8) | (value >> 24);
	}

	struct utf8_counter
	{
		typedef size_t value_type;

		static value_type low(value_type result, uint32_t ch)
		{
			// U+0000..U+007F
			if (ch < 0x80) return result + 1;
			// U+0080..U+07FF
			else if (ch < 0x800) return result + 2;
			// U+0800..U+FFFF
			else return result + 3;
		}

		static value_type high(value_type result, uint32_t)
		{
			// U+10000..U+10FFFF
			return result + 4;
		}
	};

	struct utf8_writer
	{
		typedef uint8_t* value_type;

		static value_type low(value_type result, uint32_t ch)
		{
			// U+0000..U+007F
			if (ch < 0x80)
			{
				*result = static_cast<uint8_t>(ch);
				return result + 1;
			}
			// U+0080..U+07FF
			else if (ch < 0x800)
			{
				result[0] = static_cast<uint8_t>(0xC0 | (ch >> 6));
				result[1] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
				return result + 2;
			}
			// U+0800..U+FFFF
			else
			{
				result[0] = static_cast<uint8_t>(0xE0 | (ch >> 12));
				result[1] = static_cast<uint8_t>(0x80 | ((ch >> 6) & 0x3F));
				result[2] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
				return result + 3;
			}
		}

		static value_type high(value_type result, uint32_t ch)
		{
			// U+10000..U+10FFFF
			result[0] = static_cast<uint8_t>(0xF0 | (ch >> 18));
			result[1] = static_cast<uint8_t>(0x80 | ((ch >> 12) & 0x3F));
			result[2] = static_cast<uint8_t>(0x80 | ((ch >> 6) & 0x3F));
			result[3] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
			return result + 4;
		}

		static value_type any(value_type result, uint32_t ch)
		{
			return (ch < 0x10000) ? low(result, ch) : high(result, ch);
		}
	};

	struct utf16_counter
	{
		typedef size_t value_type;

		static value_type low(value_type result, uint32_t)
		{
			return result + 1;
		}

		static value_type high(value_type result, uint32_t)
		{
			return result + 2;
		}
	};

	struct utf16_writer
	{
		typedef uint16_t* value_type;

		static value_type low(value_type result, uint32_t ch)
		{
			*result = static_cast<uint16_t>(ch);

			return result + 1;
		}

		static value_type high(value_type result, uint32_t ch)
		{
			uint32_t msh = static_cast<uint32_t>(ch - 0x10000) >> 10;
			uint32_t lsh = static_cast<uint32_t>(ch - 0x10000) & 0x3ff;

			result[0] = static_cast<uint16_t>(0xD800 + msh);
			result[1] = static_cast<uint16_t>(0xDC00 + lsh);

			return result + 2;
		}

		static value_type any(value_type result, uint32_t ch)
		{
			return (ch < 0x10000) ? low(result, ch) : high(result, ch);
		}
	};

	struct utf32_counter
	{
		typedef size_t value_type;

		static value_type low(value_type result, uint32_t)
		{
			return result + 1;
		}

		static value_type high(value_type result, uint32_t)
		{
			return result + 1;
		}
	};

	struct utf32_writer
	{
		typedef uint32_t* value_type;

		static value_type low(value_type result, uint32_t ch)
		{
			*result = ch;

			return result + 1;
		}

		static value_type high(value_type result, uint32_t ch)
		{
			*result = ch;

			return result + 1;
		}

		static value_type any(value_type result, uint32_t ch)
		{
			*result = ch;

			return result + 1;
		}
	};

	struct latin1_writer
	{
		typedef uint8_t* value_type;

		static value_type low(value_type result, uint32_t ch)
		{
			*result = static_cast<uint8_t>(ch > 255 ? '?' : ch);

			return result + 1;
		}

		static value_type high(value_type result, uint32_t ch)
		{
			(void)ch;

			*result = '?';

			return result + 1;
		}
	};

	template <size_t size> struct wchar_selector;

	template <> struct wchar_selector<2>
	{
		typedef uint16_t type;
		typedef utf16_counter counter;
		typedef utf16_writer writer;
	};

	template <> struct wchar_selector<4>
	{
		typedef uint32_t type;
		typedef utf32_counter counter;
		typedef utf32_writer writer;
	};

	typedef wchar_selector<sizeof(wchar_t)>::counter wchar_counter;
	typedef wchar_selector<sizeof(wchar_t)>::writer wchar_writer;

	template <typename Traits, typename opt_swap = opt_false> struct utf_decoder
	{
		static inline typename Traits::value_type decode_utf8_block(const uint8_t* data, size_t size, typename Traits::value_type result)
		{
			const uint8_t utf8_byte_mask = 0x3f;

			while (size)
			{
				uint8_t lead = *data;

				// 0xxxxxxx -> U+0000..U+007F
				if (lead < 0x80)
				{
					result = Traits::low(result, lead);
					data += 1;
					size -= 1;

					// process aligned single-byte (ascii) blocks
					if ((reinterpret_cast<uintptr_t>(data) & 3) == 0)
					{
						// round-trip through void* to silence 'cast increases required alignment of target type' warnings
						while (size >= 4 && (*static_cast<const uint32_t*>(static_cast<const void*>(data)) & 0x80808080) == 0)
						{
							result = Traits::low(result, data[0]);
							result = Traits::low(result, data[1]);
							result = Traits::low(result, data[2]);
							result = Traits::low(result, data[3]);
							data += 4;
							size -= 4;
						}
					}
				}
				// 110xxxxx -> U+0080..U+07FF
				else if (static_cast<unsigned int>(lead - 0xC0) < 0x20 && size >= 2 && (data[1] & 0xc0) == 0x80)
				{
					result = Traits::low(result, ((lead & ~0xC0) << 6) | (data[1] & utf8_byte_mask));
					data += 2;
					size -= 2;
				}
				// 1110xxxx -> U+0800-U+FFFF
				else if (static_cast<unsigned int>(lead - 0xE0) < 0x10 && size >= 3 && (data[1] & 0xc0) == 0x80 && (data[2] & 0xc0) == 0x80)
				{
					result = Traits::low(result, ((lead & ~0xE0) << 12) | ((data[1] & utf8_byte_mask) << 6) | (data[2] & utf8_byte_mask));
					data += 3;
					size -= 3;
				}
				// 11110xxx -> U+10000..U+10FFFF
				else if (static_cast<unsigned int>(lead - 0xF0) < 0x08 && size >= 4 && (data[1] & 0xc0) == 0x80 && (data[2] & 0xc0) == 0x80 && (data[3] & 0xc0) == 0x80)
				{
					result = Traits::high(result, ((lead & ~0xF0) << 18) | ((data[1] & utf8_byte_mask) << 12) | ((data[2] & utf8_byte_mask) << 6) | (data[3] & utf8_byte_mask));
					data += 4;
					size -= 4;
				}
				// 10xxxxxx or 11111xxx -> invalid
				else
				{
					data += 1;
					size -= 1;
				}
			}

			return result;
		}

		static inline typename Traits::value_type decode_utf16_block(const uint16_t* data, size_t size, typename Traits::value_type result)
		{
			const uint16_t* end = data + size;

			while (data < end)
			{
				uint16_t lead = opt_swap::value ? endian_swap(*data) : *data;

				// U+0000..U+D7FF
				if (lead < 0xD800)
				{
					result = Traits::low(result, lead);
					data += 1;
				}
				// U+E000..U+FFFF
				else if (static_cast<unsigned int>(lead - 0xE000) < 0x2000)
				{
					result = Traits::low(result, lead);
					data += 1;
				}
				// surrogate pair lead
				else if (static_cast<unsigned int>(lead - 0xD800) < 0x400 && data + 1 < end)
				{
					uint16_t next = opt_swap::value ? endian_swap(data[1]) : data[1];

					if (static_cast<unsigned int>(next - 0xDC00) < 0x400)
					{
						result = Traits::high(result, 0x10000 + ((lead & 0x3ff) << 10) + (next & 0x3ff));
						data += 2;
					}
					else
					{
						data += 1;
					}
				}
				else
				{
					data += 1;
				}
			}

			return result;
		}

		static inline typename Traits::value_type decode_utf32_block(const uint32_t* data, size_t size, typename Traits::value_type result)
		{
			const uint32_t* end = data + size;

			while (data < end)
			{
				uint32_t lead = opt_swap::value ? endian_swap(*data) : *data;

				// U+0000..U+FFFF
				if (lead < 0x10000)
				{
					result = Traits::low(result, lead);
					data += 1;
				}
				// U+10000..U+10FFFF
				else
				{
					result = Traits::high(result, lead);
					data += 1;
				}
			}

			return result;
		}

		static inline typename Traits::value_type decode_latin1_block(const uint8_t* data, size_t size, typename Traits::value_type result)
		{
			for (size_t i = 0; i < size; ++i)
			{
				result = Traits::low(result, data[i]);
			}

			return result;
		}

		static inline typename Traits::value_type decode_wchar_block_impl(const uint16_t* data, size_t size, typename Traits::value_type result)
		{
			return decode_utf16_block(data, size, result);
		}

		static inline typename Traits::value_type decode_wchar_block_impl(const uint32_t* data, size_t size, typename Traits::value_type result)
		{
			return decode_utf32_block(data, size, result);
		}

		static inline typename Traits::value_type decode_wchar_block(const wchar_t* data, size_t size, typename Traits::value_type result)
		{
			return decode_wchar_block_impl(reinterpret_cast<const wchar_selector<sizeof(wchar_t)>::type*>(data), size, result);
		}
	};

	template <typename T> void convert_utf_endian_swap(T* result, const T* data, size_t length)
	{
		for (size_t i = 0; i < length; ++i) result[i] = endian_swap(data[i]);
	}

#ifdef UTF_CONVERT_WCHAR_MODE
	inline void convert_wchar_endian_swap(wchar_t* result, const wchar_t* data, size_t length)
	{
		for (size_t i = 0; i < length; ++i) result[i] = static_cast<wchar_t>(endian_swap(static_cast<wchar_selector<sizeof(wchar_t)>::type>(data[i])));
	}
#endif

#ifdef UTF_CONVERT_WCHAR_MODE
	inline bool convert_buffer_utf8(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size)
	{
		const uint8_t* data = static_cast<const uint8_t*>(contents);

		// first pass: get length in wchar_t units
		out_length = utf_decoder<wchar_counter>::decode_utf8_block(data, size, 0);

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// second pass: convert utf8 input to wchar_t
		wchar_writer::value_type out_begin = reinterpret_cast<wchar_writer::value_type>(out_buffer);
		wchar_writer::value_type out_end = utf_decoder<wchar_writer>::decode_utf8_block(data, size, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}

	template <typename opt_swap> bool convert_buffer_utf16(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, opt_swap)
	{
		const uint16_t* data = static_cast<const uint16_t*>(contents);
		size_t length = size / sizeof(uint16_t);

		// first pass: get length in wchar_t units
		out_length = utf_decoder<wchar_counter, opt_swap>::decode_utf16_block(data, length, 0);

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// second pass: convert utf16 input to wchar_t
		wchar_writer::value_type out_begin = reinterpret_cast<wchar_writer::value_type>(out_buffer);
		wchar_writer::value_type out_end = utf_decoder<wchar_writer, opt_swap>::decode_utf16_block(data, length, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}

	template <typename opt_swap> bool convert_buffer_utf32(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, opt_swap)
	{
		const uint32_t* data = static_cast<const uint32_t*>(contents);
		size_t length = size / sizeof(uint32_t);

		// first pass: get length in wchar_t units
		out_length = utf_decoder<wchar_counter, opt_swap>::decode_utf32_block(data, length, 0);

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// second pass: convert utf32 input to wchar_t
		wchar_writer::value_type out_begin = reinterpret_cast<wchar_writer::value_type>(out_buffer);
		wchar_writer::value_type out_end = utf_decoder<wchar_writer, opt_swap>::decode_utf32_block(data, length, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}

	inline bool convert_buffer_latin1(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size)
	{
		const uint8_t* data = static_cast<const uint8_t*>(contents);

		// get length in wchar_t units
		out_length = size;

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// convert latin1 input to wchar_t
		wchar_writer::value_type out_begin = reinterpret_cast<wchar_writer::value_type>(out_buffer);
		wchar_writer::value_type out_end = utf_decoder<wchar_writer>::decode_latin1_block(data, size, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}
#else // no UTF_CONVERT_WCHAR_MODE
	template <typename opt_swap> bool convert_buffer_utf16(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, opt_swap)
	{
		const uint16_t* data = static_cast<const uint16_t*>(contents);
		size_t length = size / sizeof(uint16_t);

		// first pass: get length in utf8 units
		out_length = utf_decoder<utf8_counter, opt_swap>::decode_utf16_block(data, length, 0);

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// second pass: convert utf16 input to utf8
		uint8_t* out_begin = reinterpret_cast<uint8_t*>(out_buffer);
		uint8_t* out_end = utf_decoder<utf8_writer, opt_swap>::decode_utf16_block(data, length, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}

	template <typename opt_swap> bool convert_buffer_utf32(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, opt_swap)
	{
		const uint32_t* data = static_cast<const uint32_t*>(contents);
		size_t length = size / sizeof(uint32_t);

		// first pass: get length in utf8 units
		out_length = utf_decoder<utf8_counter, opt_swap>::decode_utf32_block(data, length, 0);

		// allocate buffer of suitable length
		out_buffer = static_cast<char_t*>(UTF_CONVERT_ALLOCATE((out_length > 0 ? out_length : 1) * sizeof(char_t)));
		if (!out_buffer) return false;

		// second pass: convert utf32 input to utf8
		uint8_t* out_begin = reinterpret_cast<uint8_t*>(out_buffer);
		uint8_t* out_end = utf_decoder<utf8_writer, opt_swap>::decode_utf32_block(data, length, out_begin);

		assert(out_end == out_begin + out_length);
		(void)!out_end;

		return true;
	}

	inline size_t get_latin1_7bit_prefix_length(const uint8_t* data, size_t size)
	{
		for (size_t i = 0; i < size; ++i)
			if (data[i] > 127)
				return i;

		return size;
	}
#endif

	inline size_t as_utf8_begin(const wchar_t* str, size_t length)
	{
		// get length in utf8 characters
		return utf_decoder<utf8_counter>::decode_wchar_block(str, length, 0);
	}

	inline void as_utf8_end(char* buffer, size_t size, const wchar_t* str, size_t length)
	{
		// convert to utf8
		uint8_t* begin = reinterpret_cast<uint8_t*>(buffer);
		uint8_t* end = utf_decoder<utf8_writer>::decode_wchar_block(str, length, begin);
	
		assert(begin + size == end);
		(void)!end;

		// zero-terminate
		buffer[size] = 0;
	}
	
	inline std::string as_utf8_impl(const wchar_t* str, size_t length)
	{
		// first pass: get length in utf8 characters
		size_t size = as_utf8_begin(str, length);

		// allocate resulting string
		std::string result;
		result.resize(size);

		// second pass: convert to utf8
		if (size > 0) as_utf8_end(&result[0], size, str, length);

		return result;
	}

	inline std::basic_string<wchar_t> as_wide_impl(const char* str, size_t size)
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(str);

		// first pass: get length in wchar_t units
		size_t length = utf_decoder<wchar_counter>::decode_utf8_block(data, size, 0);

		// allocate resulting string
		std::basic_string<wchar_t> result;
		result.resize(length);

		// second pass: convert to wchar_t
		if (length > 0)
		{
			wchar_writer::value_type begin = reinterpret_cast<wchar_writer::value_type>(&result[0]);
			wchar_writer::value_type end = utf_decoder<wchar_writer>::decode_utf8_block(data, size, begin);

			assert(begin + length == end);
			(void)!end;
		}

		return result;
	}

	std::string inline as_utf8(const wchar_t* str)
	{
		assert(str);

		return as_utf8_impl(str, wcslen(str));
	}

	std::string inline as_utf8(const std::basic_string<wchar_t>& str)
	{
		return as_utf8_impl(str.c_str(), str.size());
	}

	std::basic_string<wchar_t> inline as_wide(const char* str)
	{
		assert(str);

		return as_wide_impl(str, strlen(str));
	}

	std::basic_string<wchar_t> inline as_wide(const std::string& str)
	{
		return as_wide_impl(str.c_str(), str.size());
	}
} // namespace utf_convert

/**
 * Copyright (c) 2006-2012 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
