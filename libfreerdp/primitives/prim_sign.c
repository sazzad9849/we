/* FreeRDP: A Remote Desktop Protocol Client
 * Sign operations.
 * vi:ts=4 sw=4:
 *
 * (c) Copyright 2012 Hewlett-Packard Development Company, L.P.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <config.h>
#include <string.h>
#include <freerdp/types.h>
#include <freerdp/primitives.h>
#ifdef WITH_SSE2
# include <emmintrin.h>
# include <tmmintrin.h>
#endif /* WITH_SSE2 */
#include "prim_internal.h"

/* ----------------------------------------------------------------------------
 * Set pDst to the sign-value of the 16-bit values in pSrc (-1, 0, or 1).
 */
PRIM_STATIC pstatus_t general_sign_16s(
	const INT16 *pSrc,
	INT16 *pDst,
	INT32 len)
{
	while (len--)
	{
		INT16 src = *pSrc++;
		*pDst++ = (src < 0) ? (-1) : ((src > 0) ? 1 : 0);
	}

	return PRIMITIVES_SUCCESS;
}

#ifdef WITH_SSE2
/* ------------------------------------------------------------------------- */
PRIM_STATIC pstatus_t ssse3_sign_16s(
	const INT16 *pSrc,
	INT16 *pDst,
	INT32 len)
{
	const INT16 *sptr = (const INT16 *) pSrc;
	INT16 *dptr = (INT16 *) pDst;
	size_t count;

	if (len < 16)
	{
		return general_sign_16s(pSrc, pDst, len);
	}

	/* Check for 16-byte alignment (eventually). */
	if ((ULONG_PTR) pDst & 0x01)
	{
		return general_sign_16s(pSrc, pDst, len);
	}

	/* Seek 16-byte alignment. */
	while ((ULONG_PTR) dptr & 0x0f)
	{
		INT16 src = *sptr++;
		*dptr++ = (src < 0) ? (-1) : ((src > 0) ? 1 : 0);
		if (--len == 0) return PRIMITIVES_SUCCESS;
	}

	/* Do 32-short chunks using 8 XMM registers. */
	count = len >> 5;	/* / 32  */
	len -= count << 5;	/* * 32 */
	if ((ULONG_PTR) sptr & 0x0f)
	{
		/* Unaligned */
		while (count--)
		{
			__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
			xmm0 = _mm_set1_epi16(0x0001U);
			xmm1 = _mm_set1_epi16(0x0001U);
			xmm2 = _mm_set1_epi16(0x0001U);
			xmm3 = _mm_set1_epi16(0x0001U);
			xmm4 = _mm_lddqu_si128((__m128i *) sptr); sptr += 8;
			xmm5 = _mm_lddqu_si128((__m128i *) sptr); sptr += 8;
			xmm6 = _mm_lddqu_si128((__m128i *) sptr); sptr += 8;
			xmm7 = _mm_lddqu_si128((__m128i *) sptr); sptr += 8;
			xmm0 = _mm_sign_epi16(xmm0, xmm4);
			xmm1 = _mm_sign_epi16(xmm1, xmm5);
			xmm2 = _mm_sign_epi16(xmm2, xmm6);
			xmm3 = _mm_sign_epi16(xmm3, xmm7);
			_mm_store_si128((__m128i *) dptr, xmm0);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm1);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm2);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm3);         dptr += 8;
		}
	}
	else
	{
		/* Aligned */
		while (count--)
		{
			__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
			xmm0 = _mm_set1_epi16(0x0001U);
			xmm1 = _mm_set1_epi16(0x0001U);
			xmm2 = _mm_set1_epi16(0x0001U);
			xmm3 = _mm_set1_epi16(0x0001U);
			xmm4 = _mm_load_si128((__m128i *) sptr); sptr += 8;
			xmm5 = _mm_load_si128((__m128i *) sptr); sptr += 8;
			xmm6 = _mm_load_si128((__m128i *) sptr); sptr += 8;
			xmm7 = _mm_load_si128((__m128i *) sptr); sptr += 8;
			xmm0 = _mm_sign_epi16(xmm0, xmm4);
			xmm1 = _mm_sign_epi16(xmm1, xmm5);
			xmm2 = _mm_sign_epi16(xmm2, xmm6);
			xmm3 = _mm_sign_epi16(xmm3, xmm7);
			_mm_store_si128((__m128i *) dptr, xmm0);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm1);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm2);         dptr += 8;
			_mm_store_si128((__m128i *) dptr, xmm3);         dptr += 8;
		}
	}

	/* Do 8-short chunks using two XMM registers. */
	count = len >> 3;
	len -= count << 3;
	while (count--)
	{
		__m128i xmm0 = _mm_set1_epi16(0x0001U);
		__m128i xmm1 = LOAD_SI128(sptr);					sptr += 8;
		xmm0 = _mm_sign_epi16(xmm0, xmm1);
		_mm_store_si128((__m128i *) dptr, xmm0);			dptr += 8;
	}

	/* Do leftovers. */
	while (len--)
	{
		INT16 src = *sptr++;
		*dptr++ = (src < 0) ? -1 : ((src > 0) ? 1 : 0);
	}

	return PRIMITIVES_SUCCESS;
}
#endif /* WITH_SSE2 */

/* ------------------------------------------------------------------------- */
void primitives_init_sign(
	const primitives_hints_t *hints,
	primitives_t *prims)
{
	/* Start with the default. */
	prims->sign_16s = general_sign_16s;
	/* Pick tuned versions if possible. */
	/* I didn't spot an IPP version of this. */
#if defined(WITH_SSE2)
	if ((hints->x86_flags & PRIM_X86_SSSE3_AVAILABLE)
			&& (hints->x86_flags & PRIM_X86_SSE3_AVAILABLE))
	{
		prims->sign_16s  = ssse3_sign_16s;
	}
#endif
}

/* ------------------------------------------------------------------------- */
void primitives_deinit_sign(
	primitives_t *prims)
{
	/* Nothing to do. */
}
