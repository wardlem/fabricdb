/*****************************************************************
 * FabricDB Byte Order Routines
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 * 
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 * 
 * Created: January 29, 2016
 * Modified: January 29, 2016
 * Author: Mark Wardle
 * Description:
 *     Declares operations for manipulating the ordering of bytes.
 * Credit: This file is an adaptation or the byteorder routines used by Redis.
 *
 ******************************************************************/
#ifndef __FABRICDB_BYTEORDER_H
#define __FABRICDB_BYTEORDER_H

#include <stdint.h>
#include <sys/types.h>

#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define PDP_ENDIAN 3412

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER	BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif
#endif
#endif

#if !defined(BYTE_ORDER) || (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN)
#error Could not determine machine endianness.
#endif

void memrev16(void *p);
void memrev32(void *p);
void memrev64(void *p);
uint16_t intrev16(uint16_t v);
uint32_t intrev32(uint32_t v);
uint64_t intrev64(uint64_t v);

#if (BYTE_ORDER == LITTLE_ENDIAN)

#define htobeu16(v) intrev16(v)
#define htobei16(v) (int16_t) intrev16((uint16_t)v)
#define htobeu32(v) intrev32(v)
#define htobei32(v) (int32_t) intrev32((uint32_t)v)
#define htobeu64(v) memrev64(v)
#define htobei64(v) (int64_t) memrev64((uint64_t)v)

#define betohu16(v) intrev16(v)
#define betohi16(v) (int16_t) intrev16((uint16_t)v)
#define betohu32(v) intrev32(v)
#define betohi32(v) (int32_t) intrev32((uint32_t)v)
#define betohu64(v) memrev64(v)
#define betohi64(v) (int64_t) memrev64((uint64_t)v)

#define htoleu16(v) v
#define htolei16(v) v
#define htoleu32(v) v
#define htolei32(v) v
#define htoleu64(v) v
#define htolei64(v) v

#define letohu16(v) v
#define letohi16(v) v
#define letohu32(v) v
#define letohi32(v) v
#define letohu64(v) v
#define letohi64(v) v

#else

#define htobeu16(v) v
#define htobei16(v) v
#define htobeu32(v) v
#define htobei32(v) v
#define htobeu64(v) v
#define htobei64(v) v

#define betohu16(v) v
#define betohi16(v) v
#define betohu32(v) v
#define betohi32(v) v
#define betohu64(v) v
#define betohi64(v) v

#define htoleu16(v) intrev16(v)
#define htolei16(v) (int16_t) intrev16((uint16_t)v)
#define htoleu32(v) intrev32(v)
#define htolei32(v) (int32_t) intrev32((uint32_t)v)
#define htoleu64(v) intrev64(v)
#define htolei64(v) (int64_t) memrev64((uint64_t)v)

#define letohu16(v) intrev16(v)
#define letohi16(v) (int16_t) intrev16((uint16_t)v)
#define letohu32(v) intrev32(v)
#define letohi32(v) (int32_t) intrev32((uint32_t)v)
#define letohu64(v) intrev64(v)
#define letohi64(v) (int64_t) memrev64((uint64_t)v)

#endif

#endif /* __FABRICDB_BYTEORDER_H */
