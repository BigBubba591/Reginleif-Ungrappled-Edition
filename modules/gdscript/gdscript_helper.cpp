/**************************************************************************/
/*  gdscript_helper.cpp                                                   */
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

#include "gdscript_helper.h"

#include "core/variant/container_type_validate.h"
#include "core/variant/variant.h"

namespace GDScriptHelper {

static ContainerTypeValidate _to_container_type_validate(const GDScriptDataType &p_type) {
	ContainerTypeValidate ct;
	ct.variant_type = p_type.builtin_type;
	ct.class_name = p_type.native_type;
	ct.script = p_type.script_type;

	if (p_type.kind != GDScriptDataType::BUILTIN || !p_type.has_container_element_types()) {
		return ct;
	}

	int nested_count = p_type.container_element_types.size();
	ct.nested_types.resize(nested_count);
	ContainerTypeValidate *nested_ptr = ct.nested_types.ptrw();
	for (int i = 0; i < nested_count; i++) {
		nested_ptr[i] = _to_container_type_validate(p_type.container_element_types[i]);
	}

	return ct;
}

Variant construct_empty_value_for_data_type(const GDScriptDataType &p_data_type) {
	if (p_data_type.kind == GDScriptDataType::BUILTIN) {
		if (p_data_type.builtin_type == Variant::ARRAY) {
			Array array;
			// Typed array.
			if (p_data_type.has_container_element_type(0)) {
				const GDScriptDataType &element_type = p_data_type.get_container_element_type(0);
				if (element_type.has_container_element_types()) {
					array.set_typed_nested(_to_container_type_validate(element_type));
				} else {
					array.set_typed(element_type.builtin_type, element_type.native_type, element_type.script_type);
				}
			}

			return array;
		} else if (p_data_type.builtin_type == Variant::DICTIONARY) {
			Dictionary dict;
			// Typed dictionary.
			if (p_data_type.has_container_element_types()) {
				const GDScriptDataType &key_type = p_data_type.get_container_element_type_or_variant(0);
				const GDScriptDataType &value_type = p_data_type.get_container_element_type_or_variant(1);
				if (key_type.has_container_element_types() || value_type.has_container_element_types()) {
					dict.set_typed_nested(_to_container_type_validate(key_type), _to_container_type_validate(value_type));
				} else {
					dict.set_typed(key_type.builtin_type, key_type.native_type, key_type.script_type, value_type.builtin_type, value_type.native_type, value_type.script_type);
				}
			}

			return dict;
		} else {
			Callable::CallError ce;
			Variant variant;
			Variant::construct(p_data_type.builtin_type, variant, nullptr, 0, ce);

			ERR_FAIL_COND_V(ce.error != Callable::CallError::CALL_OK, Variant());

			return variant;
		}
	}

	return Variant();
}

} //namespace GDScriptHelper
