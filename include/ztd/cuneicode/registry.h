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

#ifndef ZTD_CUNEICODE_REGISTRY_H
#define ZTD_CUNEICODE_REGISTRY_H

#pragma once

#include <ztd/cuneicode/version.h>

#include <ztd/cuneicode/mcerror.h>
#include <ztd/cuneicode/open_error.h>
#include <ztd/cuneicode/registry_options.h>
#include <ztd/cuneicode/heap.h>
#include <ztd/idk/charN_t.h>

#if ZTD_IS_ON(ZTD_CXX_I_)
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#endif

//////
/// @addtogroup ztd_cuneicode_registry Conversion Registry Functions and Types
///
/// @{
//////

//////
/// @brief A structure which tracks information about the final opened cnc_conversion handle.
///
/// @remarks This structure is the only time the collection of cnc_conversion creating and opening
/// functions will return information about whether or not it uses an indirect conversion and that
/// conversion's properties.
//////
typedef struct cnc_conversion_info {
	//////
	/// @brief A pointer to the the `from_code` data.
	///
	/// @remarks This will always be a null-terminated pointer, but does not guarantee there may
	/// not be embedded nulls.
	//////
	const ztd_char8_t* from_code_data;
	//////
	/// @brief The size of the `from_code` data.
	size_t from_code_size;
	//////
	/// @brief A pointer to the the `to_code` data.
	///
	/// @remarks This will always be a null-terminated pointer, but does not guarantee there may
	/// not be embedded nulls.
	//////
	const ztd_char8_t* to_code_data;
	//////
	/// @brief The size of the `to_code` data.
	size_t to_code_size;
	//////
	/// @brief Whether or not this conversion uses an indirect conversion.
	///
	/// @remarks An indirect conversion goes through an intermediate encoding to reach it's final
	/// destination. This is typical for most conversions which encoding to and from some form of
	/// Unicode, but do not translate to each other.
	///////
	bool is_indirect;
	//////
	/// @brief The name of the indirect encoding.
	///
	/// @remarks This is `nullptr` if is_indirect is false. If it is not `nullptr`, this will be a
	/// null-terminated pointer. But, it does not guarantee there may not be embedded nulls.
	//////
	const ztd_char8_t* indirect_code_data;
	//////
	/// @brief The size of the `indirect_code` data.
	///
	/// @remarks This is 0 if is_indirect is false.
	//////
	size_t indirect_code_size;
} cnc_conversion_info;

//////
/// @brief The handle type for a conversion registry. It is an incomplete type, so it must be used
/// as `cnc_conversion_registry*` where possible.
///
/// @remarks The conversion registry is the sole owner of all of the conversion pairs added to it.
/// Each conversion registry is different when first created and does not share information with any
/// other registry. To now which operations are thread safe, check each of the registry's functions.
//////
struct cnc_conversion_registry;
//////
/// @brief A `typedef` to allow for plain and normal usage of the type name
/// `cnc_conversion_registry`.
typedef struct cnc_conversion_registry cnc_conversion_registry;

//////
/// @brief The handle type for a one-way conversion, which includes both a single conversion routine
/// and a multiple conversion routine. It is an incomplete type, so it must be used as a
/// `cnc_conversion*` where possible.
///
/// @remarks Conversion objects refer to `const` data and only modify what they themselves have
/// allocated through their `open` function. Therefore, it is safe to use a single conversion object
/// from multiple threads, provided all of the input and output data are not shared amongst other
/// threads and arguments are properly separated. See the `cnc_conv_*` functions for details.
//////
struct cnc_conversion;
//////
/// @brief A `typedef` to allow for plain and normal usage of the type name `cnc_conversion`.
typedef struct cnc_conversion cnc_conversion;

//////
/// @brief The open function type that provides a means to both measure and actually create any
/// additional state associated with the cnc_conversion object.
///
/// @param[in] __registry The conversion registry that is fielding this open request.
/// @param[in] __conversion The conversion object that will be provided. When called to just perform
/// sizing adjustments, this argument will be `nullptr`. Otherwise, it will be a pointer to a valid
/// conversion object.
/// @param[in, out] __p_available_space The number of bytes available in *__p_space, or the maximum
/// number of bytes that could be potentially used up. If `__conversion` is `nullptr`, than the
/// amount subtracted from this should be the ideal and aligned byte size of space to use.
/// @param[in, out] __p_max_alignment The required alignment for any additional state to be opened
/// up. This is ncecessary to ensure that the data has enough space properly allocated for it. Set
/// it to the requested maximum alignment when `__conversion` is `nullptr`.
/// @param[in, out] __p_space The space to write the conversion state into. This is `nullptr` if it
/// is simply requesting that the `__p_available_space` and `__p_max_alignment` arguments are to be
/// filled out and returned.
///
/// @remarks The open function is generally called twice. The first to to ask for the amount of
/// space an implementation that wishes to store additional information may need. The second is to
/// actually have it use said allocated space. The way to tell if there is space to use is to check
/// if the `__conversion` argument is `nullptr`. The second invocation can be considered like, in
/// C++ terms, wanting to invoke the constructor on any additional data that the provider of the
/// cnc_open_function wishes to store with the cnc_conversion object.
//////
typedef cnc_open_error(cnc_open_function)(cnc_conversion_registry* __registry,
     cnc_conversion* __conversion, size_t* __p_available_space, size_t* __p_max_alignment,
     void** __p_space);

//////
/// @brief The functiont type for closing the state associated with a cnc_conversion object.
///
/// @param[in] __data A pointer to the data used by the cnc_conversion object. Must be interpreted
/// in the same way its paired cnc_open_function has.
///
/// @remarks The close function is meant to clean up any state that is associated with a given
/// cnc_conversion object. This does not necessarily mean deletion: this is for when certain
/// function needs to be called or state to be cleaned up, or any dynamic allocations made by the
/// state to be deleted and released. It is not strictly for deleting the data itself. (In C++, this
/// is equivalent to invoking a destructor on the data stored by the cnc_open_function.)
//////
typedef void(cnc_close_function)(void* __data);

//////
/// @brief The function type for converting a series of bytes to another series of bytes.
///
/// @param[in] __conversion The cnc_conversion handle that called this function.
/// @param[in, out] __p_output_bytes_size A pointer to the size of the output buffer. If this is
/// `nullptr`, then it will not update the count (and the output stream will automatically be
/// considered large enough to handle all data, if
/// `__p_output_bytes` is not `nullptr`).
/// @param[in, out] __p_output_bytes A pointer to the pointer of the output buffer. If this or the
/// pointer within are `nullptr`, than this function will not write output data (it may still
/// decrement the value pointed to by
/// `__p_output_bytes_size`).
/// @param[in, out] __p_input_bytes_size A pointer to the size of the input buffer. If this is
/// `nullptr` or points to a value equivalent to `0`, then the input is considered empty and
/// CNC_MCERROR_OKAY is returned.
/// @param[in, out] __p_input_bytes A pointer to the pointer of the input buffer. If this or the
/// pointer within are `nullptr`, than the input is considered empty and CNC_MCERROR_OKAY is
/// returned.
/// @param[in] __p_state Any state allocated by the open function associated with the pair of to and
/// from names that created the cnc_conversion object that is pointed to be `__conversion`.
///
/// @remarks The conversion functions take parameters as output parameters (pointers) so that they
/// can provide information about how much of the input and output is used. Providing a `nullptr`
/// for both `__p_ouput_bytes_size` and `__p_output_bytes` serves as a way to validate the input.
/// Providing only `__p_output_bytes` but not
/// `__p_output_bytes_size` is a way to indicate that the output space is sufficiently large for the
/// input space. Providing `__p_output_bytes_size` but not `__p_output_bytes` is a way to determine
/// how much data will be written out for a given input without actually performing such a write.
//////
typedef cnc_mcerror(cnc_conversion_function)(cnc_conversion* __conversion,
     size_t* __p_output_bytes_size, unsigned char** __p_output_bytes, size_t* __p_input_bytes_size,
     const unsigned char** __p_input_bytes, void* __p_state);

//////
/// @brief The function type for iterating through a `cnc_conversion_registry`'s registered pairs of
/// conversions.
///
/// @remarks The names are given with both a pointer and a size and shall be encoded in UTF-8. The
/// names are guaranteed to be null-terminated, and therefore are suitable to pass to C-style
/// single-pointer null-terminated "c string" functions. However, it is STRONGLY recommended to use
/// the size, as embedded nulls are not strictly prohibited from being part of the UTF-8 encoded
/// name.
//////
typedef void(cnc_conversion_registry_pair_function)(size_t __from_size, const ztd_char8_t* __from,
     size_t __to_size, const ztd_char8_t* __to, void* __user_data);

//////
/// @brief Creates a new registry.
///
/// @param[in, out] __p_out_registry The output pointer to the handle of the cnc_conversion_registry
/// that will be created.
/// @param[in] __registry_options The options that affect how this registry is created.
///
/// @remarks This will default to using a normal cnc_conversion_heap which uses the
/// globally-available allocator (malloc, free, realloc, etc.).
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error cnc_new_registry(
     cnc_conversion_registry** __p_out_registry,
     cnc_registry_options __registry_options) ZTD_NOEXCEPT_IF_CXX_I_;
//////
/// @brief Creates a new registry.
///
/// @param[in, out] __p_out_registry The output pointer to the handle of the cnc_conversion_registry
/// that will be created.
/// @param[in] __p_heap A pointer to the heap to use. The heap this points to will be copied into
/// the registry upon successful creation.
/// @param[in] __registry_options The options that affect how this registry is created.
///
/// @remarks All allocations shall be done through the passed-in heap if needed. It is unspecified
/// if the implementation can or will use the heap at all (e.g., there is a small buffer
/// optimization applied).
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error cnc_open_registry(
     cnc_conversion_registry** __p_out_registry, cnc_conversion_heap* __p_heap,
     cnc_registry_options __registry_options) ZTD_NOEXCEPT_IF_CXX_I_;


//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert from. Can be `nullptr`.
/// @param[in] __to A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert to. Can be `nullptr`.
/// @param[in] __multi_conversion_function The conversion cnc_conversion_function which will perform
/// a bulk conversion (consumes as much input as is available until exhausted or an error occurs).
/// Can be `nullptr`, but only if the
/// `__single_conversion_function` is not `nullptr` as well.
/// @param[in] __single_conversion_function The conversion cnc_conversion_function which will
/// perform a singular conversion (consumes only one completely unit of input and produces on
/// complete unit of output). Can be `nullptr`, but only if the `__multi_conversion_function` is not
/// `nullptr` as well.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks This function has identical behavior to cnc_add_to_registry_n, where the `__from_size`
/// and `__to_size` arguments are calculated by calling the equivalent of `strlen` on `__from` and
/// `__to`, respectively. If `__from` or
/// `__to` are `nullptr`, then the function will assume they are the empty string (and use the
/// default name in that case).
///////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error cnc_add_to_registry(
     cnc_conversion_registry* __registry, const ztd_char8_t* __from, const ztd_char8_t* __to,
     cnc_conversion_function* __multi_conversion_function,
     cnc_conversion_function* __single_conversion_function, cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from_size A null-terminated c string encoded in UTF-8 representing the encoding to
/// convert from.
/// @param[in] __from A pointer to a string encoded in UTF-8 representing the encoding to convert
/// from. The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __to_size A pointer string encoded in UTF-8 representing the encoding to convert to.
/// @param[in] __to A pointer to a string encoded in UTF-8 representing the encoding to convert to.
/// The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __multi_conversion_function The conversion cnc_conversion_function which will perform
/// a bulk conversion (consumes as much input as is available until exhausted or an error occurs).
/// Can be `nullptr`, but only if the
/// `__single_conversion_function` is not `nullptr` as well.
/// @param[in] __single_conversion_function The conversion cnc_conversion_function which will
/// perform a singular conversion (consumes only one completely unit of input and produces on
/// complete unit of output). Can be `nullptr`, but only if the `__multi_conversion_function` is not
/// `nullptr` as well.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks Either the `__multi_conversion_function` or the `__single_conversion_function` can be
/// `nullptr`, but not both. When one is `nullptr`, the other one will be used to back-construct a
/// suitable implementation of the single-conversion function call or the multi-conversion (bulk)
/// function call, respectively. When the
/// `__open_function` or the `__close_function` are `nullptr`, they are simply defaulted when stored
/// in the registry. If
/// `__from` denotes either `nullptr` or the empty string (`__from_size == 0`), then the `__from`
/// name is defaulted to
/// `"execution"`. If `__to` denotes either `nullptr` or the empty string (`__to_size == 0`), then
/// the `__to` name is defaulted to `"UTF-8"`.
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error
cnc_add_to_registry_n(cnc_conversion_registry* __registry, size_t __from_size,
     const ztd_char8_t* __from, size_t __to_size, const ztd_char8_t* __to,
     cnc_conversion_function* __multi_conversion_function,
     cnc_conversion_function* __single_conversion_function, cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert from. Can be `nullptr`.
/// @param[in] __to A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert to. Can be `nullptr`.
/// @param[in] __multi_conversion_function The conversion cnc_conversion_function which will perform
/// a bulk conversion (consumes as much input as is available until exhausted or an error occurs).
/// Can be `nullptr`, but only if the
/// `__single_conversion_function` is not `nullptr` as well.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks Identical to calling cnc_add_to_registry_n, with the `__multi_conversion_function`
/// parameter set to `nullptr`. The `__from_size` and `__to_size` arguments are calculated by
/// calling the equivalent of `strlen` on
/// `__from` and `__to`, respectively. If `__from` or `__to` are `nullptr`, then the function will
/// assume they are the empty string (and use the default name in that case).
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error
cnc_add_to_registry_multi(cnc_conversion_registry* __registry, const ztd_char8_t* __from,
     const ztd_char8_t* __to, cnc_conversion_function* __multi_conversion_function,
     cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from_size A null-terminated c string encoded in UTF-8 representing the encoding to
/// convert from.
/// @param[in] __from A pointer to a string encoded in UTF-8 representing the encoding to convert
/// from. The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __to_size A pointer string encoded in UTF-8 representing the encoding to convert to.
/// @param[in] __to A pointer to a string encoded in UTF-8 representing the encoding to convert to.
/// The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __multi_conversion_function The conversion cnc_conversion_function which will perform
/// a bulk conversion (consumes as much input as is available until exhausted or an error occurs).
/// Can be `nullptr`, but only if the
/// `__single_conversion_function` is not `nullptr` as well.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks Identical to calling cnc_add_to_registry_n, with the `__single_conversion_function`
/// parameter set to `nullptr`.
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error
cnc_add_to_registry_n_multi(cnc_conversion_registry* __registry, size_t __from_size,
     const ztd_char8_t* __from, size_t __to_size, const ztd_char8_t* __to,
     cnc_conversion_function* __multi_conversion_function, cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert from. Can be `nullptr`.
/// @param[in] __to A pointer to a null-terminated c string encoded in UTF-8 representing the
/// encoding to convert to. Can be `nullptr`.
/// @param[in] __single_conversion_function The conversion cnc_conversion_function which will
/// perform a singular conversion (consumes only one completely unit of input and produces on
/// complete unit of output). Shall not be `nullptr`.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks Identical to calling cnc_add_to_registry_n, with the `__multi_conversion_function`
/// parameter set to `nullptr`. The `__from_size` and `__to_size` arguments are calculated by
/// calling the equivalent of `strlen` on
/// `__from` and `__to`, respectively. If `__from` or `__to` are `nullptr`, then the function will
/// assume they are the empty string (and use the default name in that case).
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error
cnc_add_to_registry_single(cnc_conversion_registry* __registry, const ztd_char8_t* __from,
     const ztd_char8_t* __to, cnc_conversion_function __single_conversion_function,
     cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Adds a new conversion from the specified `__from` and `__to` names to the specified
/// registry.
///
/// @param[in] __registry The registry to create the new conversion pair in.
/// @param[in] __from_size A null-terminated c string encoded in UTF-8 representing the encoding to
/// convert from.
/// @param[in] __from A pointer to a string encoded in UTF-8 representing the encoding to convert
/// from. The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __to_size A pointer string encoded in UTF-8 representing the encoding to convert to.
/// @param[in] __to A pointer to a string encoded in UTF-8 representing the encoding to convert to.
/// The string need not be null-terminated. Can be `nullptr`.
/// @param[in] __single_conversion_function The conversion cnc_conversion_function which will
/// perform a singular conversion (consumes only one completely unit of input and produces on
/// complete unit of output). Shall not be `nullptr`.
/// @param[in] __open_function The cnc_open_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
/// @param[in] __close_function The cnc_close_function to be used for allocating additional space
/// during function calls which open new cnc_conversion handles. Can be `nullptr`.
///
/// @remarks Identical to calling cnc_add_to_registry_n, with the `__multi_conversion_function`
/// parameter set to `nullptr`.
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ cnc_open_error
cnc_add_to_registry_n_single(cnc_conversion_registry* __registry, size_t __from_size,
     const ztd_char8_t* __from, size_t __to_size, const ztd_char8_t* __to,
     cnc_conversion_function* __single_conversion_function, cnc_open_function* __open_function,
     cnc_close_function* __close_function) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Closes an open registry.
///
/// @param[in] __registry The registry to delete.
///
/// @remarks This function MUST be paired with @ref cnc_open_registry. It cannot be paired with any
/// other creation function. This will close the registry before it's memory is deleted/freed: see
/// cnc_close_registry for more information. If `registry` is `nullptr`, this function will do
/// nothing.
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ void cnc_close_registry(
     cnc_conversion_registry* __registry) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Deletes a registry.
///
/// @param[in] __registry The registry to delete.
///
/// @remarks This function MUST be paired with @ref cnc_new_registry. It cannot be paired with any
/// other creation function. This will close the registry before it's memory is deleted/freed: see
/// cnc_close_registry for more information. If `registry` is `nullptr`, this function will do
/// nothing.
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ void cnc_delete_registry(
     cnc_conversion_registry* __registry) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @brief Provides the list of encoding conversions currently registered to the provided
/// `__registry`.
///
/// @param[in] __registry The conversion registry whose conversion pairs should be iterated through
/// and passed to the function.
/// @param[in] __callback_function The function that should be called with each conversion pair. See
/// cnc_conversion_registry_pair_function for more details about the expectation of each given
/// parameter.
/// @param[in] __user_data A pointer to data that should be given to the `__callback_function`.
///
/// @remarks This functions does not modify the contents of the registry and therefore can be called
/// from any number of threads simultaneously. The callback function is invoked once more each pair.
/// Note that each conversion pair is distinct from the others: a conversion pair of ("UTF-8",
/// "SHIFT-JIS") is distinct from ("SHIFT-JIS", "UTF-8"). If `registry` or `__callback_function` is
/// `nullptr`, this function will do nothing.
//////
ZTD_CUNEICODE_C_LANGUAGE_LINKAGE_I_ ZTD_CUNEICODE_API_LINKAGE_I_ void cnc_pairs_list(
     const cnc_conversion_registry* __registry,
     cnc_conversion_registry_pair_function __callback_function,
     void* __user_data) ZTD_NOEXCEPT_IF_CXX_I_;

//////
/// @}
//////

#endif // ZTD_CUNEICODE_REGISTRY_H