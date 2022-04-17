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

#pragma once

#ifndef ZTD_CUNEICODE_SOURCE_DETAIL_TRANSCODE_HPP
#define ZTD_CUNEICODE_SOURCE_DETAIL_TRANSCODE_HPP

#include <ztd/cuneicode/version.h>

#include <ztd/cuneicode/mcstate.h>

#include <cassert>

namespace ztd { namespace cnc {
	ZTD_CUNEICODE_INLINE_ABI_NAMESPACE_OPEN_I_

	namespace __cnc_detail {

		template <bool _IsCounting, bool _IsUnbounded, typename _FromFunc, _FromFunc __xsnrtoisn,
			typename _ToFunc, _ToFunc __isnrtozsn, typename _IntermediateChar = ztd_char32_t,
			size_t _IntermediateMax = CNC_C32_MAX, typename _SourceChar, typename _DestChar>
		cnc_mcerror __basic_transcode_one(size_t* __p_maybe_dst_len, _DestChar** __p_maybe_dst,
			size_t* __p_src_len, const _SourceChar** __p_src,
			cnc_mcstate_t* __p_state) noexcept {
			if (__p_src_len == nullptr || *__p_src_len < 1) {
				// empty sources are just fine (returns 0)
				return CNC_MCERROR_OKAY;
			}
			if (__p_src == nullptr || *__p_src == nullptr) {
				// null sources are treated like empty sources: okay
				return CNC_MCERROR_OKAY;
			}

			const _SourceChar*& __src             = *__p_src;
			size_t& __src_len                     = *__p_src_len;
			const _SourceChar* __intermediate_src = __src;
			size_t __intermediate_src_len         = __src_len;

			_IntermediateChar __intermediate[_IntermediateMax];
			_IntermediateChar* __intermediate_first = __intermediate;
			size_t __intermediate_out_size_after    = _IntermediateMax;
			cnc_mcerror __res_decode                = __xsnrtoisn(&__intermediate_out_size_after,
				               &__intermediate_first, &__intermediate_src_len, &__intermediate_src, __p_state);
			switch (__res_decode) {
			case CNC_MCERROR_INVALID_SEQUENCE:
				// error, explode
				return __res_decode;
			case CNC_MCERROR_INCOMPLETE_INPUT:
				// out of input
				// no need to change __src or __src_len
				return __res_decode;
			case CNC_MCERROR_INSUFFICIENT_OUTPUT:
				// this error should never happen, ever!
				return __res_decode;
			case CNC_MCERROR_OKAY:
			default:
				// we can keep going!
				break;
			}

			const size_t __intermediate_out_size
				= _IntermediateMax - __intermediate_out_size_after;
			const _IntermediateChar* __intermediate_input_first = __intermediate;
			size_t __intermediate_input_size_after              = __intermediate_out_size;

			cnc_mcerror __res_encode = __isnrtozsn(__p_maybe_dst_len, __p_maybe_dst,
				&__intermediate_input_size_after, &__intermediate_input_first, __p_state);
			switch (__res_encode) {
			case CNC_MCERROR_INVALID_SEQUENCE:
				// error, rollback
				return __res_encode;
			case CNC_MCERROR_INCOMPLETE_INPUT:
				// out of input
				// no need to increment __src or __src_len
				return __res_encode;
			case CNC_MCERROR_INSUFFICIENT_OUTPUT:
				// this error should never happen, ever!
				return __res_encode;
			case CNC_MCERROR_OKAY:
			default:
				// all good, update original pointers!
				__src     = __intermediate_src;
				__src_len = __intermediate_src_len;
				break;
			}
			return CNC_MCERROR_OKAY;
		}

		template <bool _IsCounting, bool _IsUnbounded, size_t _IntermediateMax, typename _Func,
			_Func __xsnrtoysn, typename _SourceChar, typename _DestChar>
		cnc_mcerror __transcode(size_t* __p_maybe_dst_len, _DestChar** __p_maybe_dst,
			size_t* __p_src_len, const _SourceChar** __p_src,
			cnc_mcstate_t* __p_state) noexcept {
			if (__p_src_len == nullptr || *__p_src_len < 1) {
				// empty sources are just fine (returns 0)
				return CNC_MCERROR_OKAY;
			}
			if (__p_src == nullptr || *__p_src == nullptr) {
				// null sources are treated like empty sources: okay
				return CNC_MCERROR_OKAY;
			}

			if constexpr (!_IsUnbounded) {
				if (*__p_maybe_dst_len < 1) {
					// we are not counting, and there is not enough output
					// spaced
					return CNC_MCERROR_INSUFFICIENT_OUTPUT;
				}
			}

			const _SourceChar*& __src = *__p_src;
			size_t& __src_len         = *__p_src_len;

			for (; __src_len > 0;) {
				cnc_mcerror __res = __xsnrtoysn(
					__p_maybe_dst_len, __p_maybe_dst, &__src_len, &__src, __p_state);
				switch (__res) {
				case CNC_MCERROR_OKAY:
					if (__src_len == 0) {
						return CNC_MCERROR_OKAY;
					}
					break;
				default:
					// error, explode
					return __res;
				}
			}
			return CNC_MCERROR_INVALID_SEQUENCE;
		}

#define _ZTDC_CUNEICODE_TRANSCODE_ONE_BODY(_DL, _D, _SL, _S, _ST, ...)                            \
	if (_D == nullptr || *_D == nullptr) {                                                       \
		if (_DL == nullptr) {                                                                   \
			return ::ztd::cnc::__cnc_detail::__basic_transcode_one<true, true, __VA_ARGS__>(   \
			     _DL, _D, _SL, _S, _ST);                                                       \
		}                                                                                       \
		else {                                                                                  \
			return ::ztd::cnc::__cnc_detail::__basic_transcode_one<true, false, __VA_ARGS__>(  \
			     _DL, _D, _SL, _S, _ST);                                                       \
		}                                                                                       \
	}                                                                                            \
	else {                                                                                       \
		if (_DL == nullptr) {                                                                   \
			return ::ztd::cnc::__cnc_detail::__basic_transcode_one<false, true, __VA_ARGS__>(  \
			     _DL, _D, _SL, _S, _ST);                                                       \
		}                                                                                       \
		else {                                                                                  \
			return ::ztd::cnc::__cnc_detail::__basic_transcode_one<false, false, __VA_ARGS__>( \
			     _DL, _D, _SL, _S, _ST);                                                       \
		}                                                                                       \
	}

	} // namespace __cnc_detail
	ZTD_CUNEICODE_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::cnc

#endif // ZTD_CUNEICODE_SOURCE_DETAIL_TRANSCODE_HPP