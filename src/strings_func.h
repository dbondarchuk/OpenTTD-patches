/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file strings_func.h Functions related to OTTD's strings. */

#ifndef STRINGS_FUNC_H
#define STRINGS_FUNC_H

#include "strings_type.h"
#include "string.h"
#include "gfx_type.h"

class StringParameters {
	StringParameters *parent; ///< If not NULL, this instance references data from this parent instance.
	uint64 *data;             ///< Array with the actual data.
	WChar *type;              ///< Array with type information about the data. Can be NULL when no type information is needed. See #StringControlCode.

public:
	uint offset;              ///< Current offset in the data/type arrays.
	uint num_param;           ///< Length of the data array.

	/** Create a new StringParameters instance. */
	StringParameters(uint64 *data, uint num_param, WChar *type) :
		parent(NULL),
		data(data),
		type(type),
		offset(0),
		num_param(num_param)
	{ }

	/** Create a new StringParameters instance. */
	template <size_t Tnum_param>
	StringParameters(int64 (&data)[Tnum_param]) :
		parent(NULL),
		data((uint64 *)data),
		type(NULL),
		offset(0),
		num_param(Tnum_param)
	{
		assert_compile(sizeof(data[0]) == sizeof(uint64));
	}

	/**
	 * Create a new StringParameters instance that can reference part of the data of
	 * the given partent instance.
	 */
	StringParameters(StringParameters &parent, uint size) :
		parent(&parent),
		data(parent.data + parent.offset),
		offset(0),
		num_param(size)
	{
		assert(size <= parent.GetDataLeft());
		if (parent.type == NULL) {
			this->type = NULL;
		} else {
			this->type = parent.type + parent.offset;
		}
	}

	~StringParameters()
	{
		if (this->parent != NULL) {
			this->parent->offset += this->num_param;
		}
	}

	void ClearTypeInformation();

	int64 GetInt64(WChar type = 0);

	/** Read an int32 from the argument array. @see GetInt64. */
	int32 GetInt32(WChar type = 0)
	{
		return (int32)this->GetInt64(type);
	}

	/** Get a pointer to the current element in the data array. */
	uint64 *GetDataPointer() const
	{
		return &this->data[this->offset];
	}

	/** Return the amount of elements which can still be read. */
	uint GetDataLeft() const
	{
		return this->num_param - this->offset;
	}

	/** Get a pointer to a specific element in the data array. */
	uint64 *GetPointerToOffset(uint offset) const
	{
		assert(offset < this->num_param);
		return &this->data[offset];
	}

	/** Does this instance store information about the type of the parameters. */
	bool HasTypeInformation() const
	{
		return this->type != NULL;
	}

	/** Get the type of a specific element. */
	WChar GetTypeAtOffset(uint offset) const
	{
		assert(offset < this->num_param);
		assert(this->HasTypeInformation());
		return this->type[offset];
	}

	void SetParam(uint n, uint64 v)
	{
		assert(n < this->num_param);
		this->data[n] = v;
	}

	uint64 GetParam(uint n) const
	{
		assert(n < this->num_param);
		return this->data[n];
	}
};
extern StringParameters _global_string_params;

const char *GetStringPtr(StringID string);

void AppendString (stringb *buf, StringID string);

static inline void GetString (stringb *buf, StringID string)
{
	buf->clear();
	AppendString (buf, string);
}

template <uint N>
static inline void GetString (char (*buf) [N], StringID string)
{
	stringb tmp (N, &(*buf)[0]);
	GetString (&tmp, string);
}

template <uint N>
static inline void GetString (char (&buf) [N], StringID string)
{
	GetString (&buf, string);
}

void AppendStringWithArgs (stringb *buf, StringID string, StringParameters *args, uint case_index = 0, bool game_script = false);

uint ConvertKmhishSpeedToDisplaySpeed(uint speed);
uint ConvertDisplaySpeedToKmhishSpeed(uint speed);

/**
 * Set a string parameter \a v at index \a n in the global string parameter array.
 * @param n Index of the string parameter.
 * @param v Value of the string parameter.
 */
static inline void SetDParam(uint n, uint64 v)
{
	_global_string_params.SetParam(n, v);
}

void SetDParamMaxValue(uint n, uint64 max_value, uint min_count = 0, FontSize size = FS_NORMAL);
void SetDParamMaxDigits(uint n, uint count, FontSize size = FS_NORMAL);

void SetDParamStr(uint n, const char *str);

void CopyInDParam(int offs, const uint64 *src, int num);
void CopyOutDParam(uint64 *dst, int offs, int num);
void CopyOutDParam(uint64 *dst, const char **strings, StringID string, int num);

/**
 * Get the current string parameter at index \a n from the global string parameter array.
 * @param n Index of the string parameter.
 * @return Value of the requested string parameter.
 */
static inline uint64 GetDParam(uint n)
{
	return _global_string_params.GetParam(n);
}

extern TextDirection _current_text_dir; ///< Text direction of the currently selected language

void InitializeLanguagePacks();
const char *GetCurrentLanguageIsoCode();

int CDECL StringIDSorter(const StringID *a, const StringID *b);

/**
 * A searcher for missing glyphs.
 */
class MissingGlyphSearcher {
public:
	const FontSize default_size; ///< default font size of the string
	const bool     monospace;    ///< whether to search for a monospace font

	/** Construct an instance. */
	CONSTEXPR MissingGlyphSearcher (FontSize size, bool mono)
		: default_size(size), monospace(mono)
	{
	}

	/** Make sure everything gets destructed right. */
	virtual ~MissingGlyphSearcher() {}

	/**
	 * Get the next string to search through.
	 * @return The next string or NULL if there is none.
	 */
	virtual const char *NextString() = 0;

	/**
	 * Get the default (font) size of the string.
	 * @return The font size.
	 */
	FontSize DefaultSize() const
	{
		return this->default_size;
	}

	/**
	 * Reset the search, i.e. begin from the beginning again.
	 */
	virtual void Reset() = 0;

	/**
	 * Whether to search for a monospace font or not.
	 * @return True if searching for monospace.
	 */
	bool Monospace() const
	{
		return this->monospace;
	}

	void SetFontNames (struct FreeTypeSettings *settings, const char *font_name);

	bool FindMissingGlyphs (void);
};

void CheckForMissingGlyphs(bool base_font = true, MissingGlyphSearcher *search = NULL);

#endif /* STRINGS_FUNC_H */
