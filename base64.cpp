
#include "base64.h"



size_t modp_b64_encode(char* dest, const char* str, size_t len)
{
	size_t i = 0;
	uint8_t* p = (uint8_t*)dest;

	/* unsigned here is important! */
	uint8_t t1, t2, t3;

	if (len > 2) {
		for (; i < len - 2; i += 3) {
			t1 = str[i]; t2 = str[i + 1]; t3 = str[i + 2];
			*p++ = e0[t1];
			*p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
			*p++ = e1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
			*p++ = e2[t3];
		}
	}

	switch (len - i) {
	case 0:
		break;
	case 1:
		t1 = str[i];
		*p++ = e0[t1];
		*p++ = e1[(t1 & 0x03) << 4];
		*p++ = CHARPAD;
		*p++ = CHARPAD;
		break;
	default: /* case 2 */
		t1 = str[i]; t2 = str[i + 1];
		*p++ = e0[t1];
		*p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
		*p++ = e2[(t2 & 0x0F) << 2];
		*p++ = CHARPAD;
	}

	*p = '\0';
	return p - (uint8_t*)dest;
}

#ifdef WORDS_BIGENDIAN   /* BIG ENDIAN -- SUN / IBM / MOTOROLA */
int modp_b64_decode(char* dest, const char* src, int len)
{
	if (len == 0) return 0;

#ifdef DOPAD
	/* if padding is used, then the message must be at least
	4 chars and be a multiple of 4.
	there can be at most 2 pad chars at the end */
	if (len < 4 || (len % 4 != 0)) return MODP_B64_ERROR;
	if (src[len - 1] == CHARPAD) {
		len--;
		if (src[len - 1] == CHARPAD) {
			len--;
		}
	}
#endif  /* DOPAD */

	size_t i;
	int leftover = len % 4;
	size_t chunks = (leftover == 0) ? len / 4 - 1 : len / 4;

	uint8_t* p = (uint8_t*)dest;
	uint32_t x = 0;
	uint32_t* destInt = (uint32_t*)p;
	uint32_t* srcInt = (uint32_t*)src;
	uint32_t y = *srcInt++;
	for (i = 0; i < chunks; ++i) {
		x = d0[y >> 24 & 0xff] | d1[y >> 16 & 0xff] |
			d2[y >> 8 & 0xff] | d3[y & 0xff];

		if (x >= BADCHAR)  return MODP_B64_ERROR;
		*destInt = x << 8;
		p += 3;
		destInt = (uint32_t*)p;
		y = *srcInt++;
	}

	switch (leftover) {
	case 0:
		x = d0[y >> 24 & 0xff] | d1[y >> 16 & 0xff] |
			d2[y >> 8 & 0xff] | d3[y & 0xff];
		if (x >= BADCHAR)  return MODP_B64_ERROR;
		*p++ = ((uint8_t*)&x)[1];
		*p++ = ((uint8_t*)&x)[2];
		*p = ((uint8_t*)&x)[3];
		return (chunks + 1) * 3;
	case 1:
		x = d3[y >> 24];
		*p = (uint8_t)x;
		break;
	case 2:
		x = d3[y >> 24] * 64 + d3[(y >> 16) & 0xff];
		*p = (uint8_t)(x >> 4);
		break;
	default:  /* case 3 */
		x = (d3[y >> 24] * 64 + d3[(y >> 16) & 0xff]) * 64 +
			d3[(y >> 8) & 0xff];
		*p++ = (uint8_t)(x >> 10);
		*p = (uint8_t)(x >> 2);
		break;
	}

	if (x >= BADCHAR) return MODP_B64_ERROR;
	return 3 * chunks + (6 * leftover) / 8;
}

#else /* LITTLE  ENDIAN -- INTEL AND FRIENDS */

size_t modp_b64_decode(char* dest, const char* src, size_t len)
{
	if (len == 0) return 0;

#ifdef DOPAD
	/*
	* if padding is used, then the message must be at least
	* 4 chars and be a multiple of 4
	*/
	if (len < 4 || (len % 4 != 0)) return MODP_B64_ERROR; /* error */
	/* there can be at most 2 pad chars at the end */
	if (src[len - 1] == CHARPAD) {
		len--;
		if (src[len - 1] == CHARPAD) {
			len--;
		}
	}
#endif

	size_t i;
	int leftover = len % 4;
	size_t chunks = (leftover == 0) ? len / 4 - 1 : len / 4;

	uint8_t* p = (uint8_t*)dest;
	uint32_t x = 0;
	uint32_t* destInt = (uint32_t*)p;
	uint32_t* srcInt = (uint32_t*)src;
	uint32_t y = *srcInt++;
	for (i = 0; i < chunks; ++i) {
		x = d0[y & 0xff] |
			d1[(y >> 8) & 0xff] |
			d2[(y >> 16) & 0xff] |
			d3[(y >> 24) & 0xff];

		if (x >= BADCHAR) return MODP_B64_ERROR;
		*destInt = x;
		p += 3;
		destInt = (uint32_t*)p;
		y = *srcInt++;
	}


	switch (leftover) {
	case 0:
		x = d0[y & 0xff] |
			d1[(y >> 8) & 0xff] |
			d2[(y >> 16) & 0xff] |
			d3[(y >> 24) & 0xff];

		if (x >= BADCHAR) return MODP_B64_ERROR;
		*p++ = ((uint8_t*)(&x))[0];
		*p++ = ((uint8_t*)(&x))[1];
		*p = ((uint8_t*)(&x))[2];
		return (chunks + 1) * 3;
		break;
	case 1:  /* with padding this is an impossible case */
		x = d0[y & 0xff];
		*p = *((uint8_t*)(&x)); // i.e. first char/byte in int
		break;
	case 2: // * case 2, 1  output byte */
		x = d0[y & 0xff] | d1[y >> 8 & 0xff];
		*p = *((uint8_t*)(&x)); // i.e. first char
		break;
	default: /* case 3, 2 output bytes */
		x = d0[y & 0xff] |
			d1[y >> 8 & 0xff] |
			d2[y >> 16 & 0xff];  /* 0x3c */
		*p++ = ((uint8_t*)(&x))[0];
		*p = ((uint8_t*)(&x))[1];
		break;
	}

	if (x >= BADCHAR) return MODP_B64_ERROR;

	return 3 * chunks + (6 * leftover) / 8;
}

#endif  /* if bigendian / else / endif */

bool Base64Encode(const std::string& input, std::string* output){
	std::string temp;
	temp.resize(modp_b64_encode_len(input.size()));
	int input_size = static_cast<int>(input.size());
	size_t output_size = modp_b64_encode(&temp[0], input.data(), input_size);
	if (output_size == MODP_B64_ERROR)
		return false;
	temp.resize(output_size);
	output->swap(temp);
	return true;
}

bool Base64Decode(const std::string& input, std::string* output){
	std::string temp;
	temp.resize(modp_b64_decode_len(input.size()));
	int input_size = static_cast<int>(input.size());
	//size_t output_size = modp_b64_decode(&temp[0],input.data(),input_size);
	//if (output_size==MODP_B64_ERROR)
	//	return false;
	//temp.resize(output_size);
	temp = base64_decode(input);
	output->swap(temp);
	return true;
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

std::string base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}