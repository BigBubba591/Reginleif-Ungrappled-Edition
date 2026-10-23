/**************************************************************************/
/*  runtime_impl_table.cpp                                                */
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

#include "runtime_impl_table.h"

#include "core/object/class_db.h"

void RuntimeImplTable::register_method(const Key& p_key, const StringName& p_method, GDScriptFunction* p_function) {
	table[p_key][p_method] = p_function;
	native_chain_cache.clear();
}

void RuntimeImplTable::unregister_function(GDScriptFunction* p_function) {
	for (HashMap<Key, MethodMap, KeyHasher>::Iterator E = table.begin(); E;) {
		HashMap<Key, MethodMap, KeyHasher>::Iterator next = E;
		++next;
		MethodMap& methods = E->value;
		for (MethodMap::Iterator M = methods.begin(); M;) {

			MethodMap::Iterator next_m = M;
			++next_m;

			if (M->value == p_function) {
				methods.remove(M);
			}

			M = next_m;
		}

		if (methods.is_empty()) {
			table.remove(E);
		}
		E = next;
	}
	native_chain_cache.clear();
}

bool RuntimeImplTable::has_any(const Key& p_key) const {
	return table.has(p_key);
}

GDScriptFunction *RuntimeImplTable::find(const Key& p_key, const StringName& p_method) const {
	HashMap<Key, MethodMap, KeyHasher>::ConstIterator E = table.find(p_key);
	if (!E) {
		return nullptr;
	}

	MethodMap::ConstIterator M = E->value.find(p_method);
	return M ? M->value : nullptr;
}

GDScriptFunction* RuntimeImplTable::find_in_native_chain(const StringName& p_concrete_class, const StringName& p_method) const {

	HashMap<StringName, MethodMap>::ConstIterator cached_class = native_chain_cache.find(p_concrete_class);

	if (cached_class) {
		MethodMap::ConstIterator cached_method = cached_class->value.find(p_method);
		if (cached_method) {
			return cached_method->value;
		}
	}

	GDScriptFunction* found = nullptr;
	for (StringName walk = p_concrete_class; walk != StringName(); walk = ClassDB::get_parent_class(walk)) {
		found = find(Key::native(walk), p_method);      ///^^^^^^^ phase out when classDB is inevitably gutted
		if (found != nullptr) {
			break;
		}
	}

	native_chain_cache[p_concrete_class][p_method] = found;
	return found;
}

void RuntimeImplTable::take_all_functions(Vector<GDScriptFunction*>& r_functions) {
	for (const KeyValue<Key, MethodMap>& E : table) {
		for (const KeyValue<StringName, GDScriptFunction*>& M : E.value) {
			if (M.value != nullptr && !r_functions.has(M.value)) {
				r_functions.push_back(M.value);
			}
		}
	}
	table.clear();
	native_chain_cache.clear();
}