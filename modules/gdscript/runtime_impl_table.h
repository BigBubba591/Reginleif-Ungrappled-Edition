/**************************************************************************/
/*  runtime_impl_table.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/
/// Hey! This file was created for the Reginleif fork! It's not part of Godot upstream.
/// Copyright (c) 2026 chesedcore (Monarch).
/// Licensed under the MIT License, same terms as the Godot engine.

#pragma once

#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/hashfuncs.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"

class GDScriptFunction;

class RuntimeImplTable {
public:
	struct Key {
		enum Kind : uint8_t {
			BUILTIN,
			NATIVE,
			ENUM,
			SCRIPT_CLASS,
		};
		Kind kind = BUILTIN;
		uint32_t id = 0;
		StringName name;

		static Key builtin(Variant::Type p_type) { return { BUILTIN, (uint32_t)p_type, StringName() }; }
		static Key native(const StringName& p_class) { return { NATIVE, 0, p_class }; }
		static Key enum_type(const StringName& p_key) { return { ENUM, 0, p_key }; }
		static Key script_class(const StringName& p_fqcn) { return { SCRIPT_CLASS, 0, p_fqcn }; }

		bool operator==(const Key& p_o) const { return kind == p_o.kind && id == p_o.id && name == p_o.name; }
		uint32_t hash() const { return hash_murmur3_one_32(((uint32_t)kind << 24) ^ id, name.hash()); }
	};

	struct KeyHasher {
		static _FORCE_INLINE_ uint32_t hash(const Key& p_k) { return p_k.hash(); }
	};

	typedef HashMap<StringName, GDScriptFunction*> MethodMap;

private:
	HashMap<Key, MethodMap, KeyHasher> table;
	mutable HashMap<StringName, MethodMap> native_chain_cache;

public:
	void register_method(const Key& p_key, const StringName& p_method, GDScriptFunction* p_function);
	void unregister_function(GDScriptFunction* p_function);
	bool has_any(const Key& p_key) const;
	GDScriptFunction* find(const Key& p_key, const StringName& p_method) const;
	GDScriptFunction* find_in_native_chain(const StringName& p_concrete_class, const StringName& p_method) const;
	void take_all_functions(Vector<GDScriptFunction*>& r_functions);
};