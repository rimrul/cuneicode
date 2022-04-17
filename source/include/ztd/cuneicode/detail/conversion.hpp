// =============================================================================
//
// ztd.cuneicode
// Copyright © 2021-2022 JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
// Contact: opensource@soasis.org
//
// Commercial License Usage
// Licensees holding valid commercial ztd.cuneicode licenses may use this file
// in accordance with the commercial license agreement provided with the
// Software or, alternatively, in accordance with the terms contained in
// a written agreement between you and Shepherd's Oasis, LLC.
// For licensing terms and conditions see your agreement. For
// further information contact opensource@soasis.org.
//
// Apache License Version 2 Usage
// Alternatively, this file may be used under the terms of Apache License
// Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// 		http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ============================================================================>

#ifndef ZTD_CUNEICODE_SOURCE_DETAIL_CONVERSION_HPP
#define ZTD_CUNEICODE_SOURCE_DETAIL_CONVERSION_HPP

#pragma once

#include <ztd/cuneicode/version.h>

#include <ztd/cuneicode/conv.h>


#if ZTD_IS_ON(ZTD_CXX_I_)
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#endif

typedef struct cnc_conversion {
	cnc_conversion_registry* __registry;
	size_t __size;
	cnc_conversion_function* __single_conversion_function;
	cnc_conversion_function* __multi_conversion_function;
	cnc_close_function* __close_function;
} cnc_conversion;

extern cnc_mcerror __cnc_multi_from_single_conversion(cnc_conversion* __conversion,
     size_t* __p_output_bytes_size, unsigned char** __p_output_bytes, size_t* __p_input_bytes_size,
     const unsigned char** __p_input_bytes, void* __user_data) ZTD_NOEXCEPT_IF_CXX_I_;
extern cnc_mcerror __cnc_single_from_multi_conversion(cnc_conversion* __conversion,
     size_t* __p_output_bytes_size, unsigned char** __p_output_bytes, size_t* __p_input_bytes_size,
     const unsigned char** __p_input_bytes, void* __user_data) ZTD_NOEXCEPT_IF_CXX_I_;

#endif // ZTD_CUNEICODE_SOURCE_DETAIL_CONVERSION_HPP