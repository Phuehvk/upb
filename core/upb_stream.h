/*
 * upb - a minimalist implementation of protocol buffers.
 *
 * This file defines four general-purpose streaming interfaces for protobuf
 * data or bytes:
 *
 * - upb_src: pull interface for protobuf data.
 * - upb_sink: push interface for protobuf data.
 * - upb_bytesrc: pull interface for bytes.
 * - upb_bytesink: push interface for bytes.
 *
 * These interfaces are used as general-purpose glue in upb.  For example, the
 * decoder interface works by implementing a upb_src and calling a upb_bytesrc.
 *
 * Copyright (c) 2010 Joshua Haberman.  See LICENSE for details.
 *
 */

#ifndef UPB_SRCSINK_H
#define UPB_SRCSINK_H

#include "upb_stream_vtbl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward-declare.  We can't include upb_def.h; it would be circular.
struct _upb_fielddef;

// Note!  The "eof" flags work like feof() in C; they cannot report end-of-file
// until a read has failed due to eof.  They cannot preemptively tell you that
// the next call will fail due to eof.  Since these are the semantics that C
// and UNIX provide, we're stuck with them if we want to support eg. stdio.

/* upb_src ********************************************************************/

// A upb_src is a pull parser for protobuf data.  Sample usage:
//
//   #define CHECK(x) if(!x) goto err;
//
//   bool parse_msg(upb_src *src, int indent) {
//     upb_fielddef *f;
//     while ((f = upb_src_getdef(src)) != NULL) {
//       for (int i = 0; i < indent; i++) putchar(' ');
//       printf("Parsed field; name=" UPB_STRFMT ", num=%d",
//              UPB_STRARG(d->name), d->number);
//       if (upb_issubmsg(f)) {
//         CHECK(upb_src_startmsg(src));
//         CHECK(parse_msg(src, indent + 2));
//         CHECK(upb_src_endmsg(src));
//       } else {
//         CHECK(upb_src_skipval(src));
//       }
//     }
//     // We should be EOF now, otherwise there was an error.
//     CHECK(upb_src_eof(src));
//     return true;
//
//   err:
//     return false;
//   }
//
// TODO: decide how to handle unknown fields.

// Retrieves the fielddef for the next field in the stream.  Returns NULL on
// error or end-of-stream.  End of stream can simply mean end of submessage.
struct _upb_fielddef *upb_src_getdef(upb_src *src);

// Retrieves and stores the next value in "val".  upb_src_getval() is for all
// numeric types and upb_src_getstr() is for strings.  For string types "str"
// must be a newly-recycled string.  Returns false on error.
bool upb_src_getval(upb_src *src, upb_valueptr val);
bool upb_src_getstr(upb_src *src, upb_string *val);

// Like upb_src_getval() but skips the value.
bool upb_src_skipval(upb_src *src);

// Descends into a submessage.  May only be called when upb_issubmsg(f) is true
// for an f = upb_src_getdef(src) that was just parsed.
bool upb_src_startmsg(upb_src *src);

// Stops reading a submessage.  May be called before the stream is EOF, in
// which case the rest of the submessage is skipped.
bool upb_src_endmsg(upb_src *src);

// Returns the current error/eof status for the stream.  If a stream is eof
// but we are inside a submessage, calling upb_src_endmsg(src) will reset
// the eof marker.
INLINE upb_status *upb_src_status(upb_src *src) { return &src->status; }
INLINE bool upb_src_eof(upb_src *src) { return src->eof; }

// The following functions are equivalent to upb_src_getval(), but take
// pointers to specific types.  In debug mode this may check that the type
// is compatible with the type being read.  This check will *not* be performed
// in non-debug mode, and if you get the type wrong the behavior is undefined.
bool upb_src_getbool(upb_src *src, bool *val);
bool upb_src_getint32(upb_src *src, int32_t *val);
bool upb_src_getint64(upb_src *src, int64_t *val);
bool upb_src_getuint32(upb_src *src, uint32_t *val);
bool upb_src_getuint64(upb_src *src, uint64_t *val);
bool upb_src_getfloat(upb_src *src, float *val);
bool upb_src_getdouble(upb_src *src, double *val);

/* upb_sink *******************************************************************/

// Puts the given fielddef into the stream.
bool upb_sink_putdef(upb_sink *sink, struct _upb_fielddef *def);

// Puts the given value into the stream.
bool upb_sink_putval(upb_sink *sink, upb_value val);
bool upb_sink_putstr(upb_sink *sink, upb_string *str);

// Starts/ends a submessage.  upb_sink_startmsg may seem redundant, but a
// client could have a submessage already serialized, and therefore put it
// as a string instead of its individual elements.
bool upb_sink_startmsg(upb_sink *sink);
bool upb_sink_endmsg(upb_sink *sink);

// Returns the current error status for the stream.
upb_status *upb_sink_status(upb_sink *sink);

/* upb_bytesrc ****************************************************************/

// Returns the next string in the stream.  false is returned on error or eof.
// The string must be at least "minlen" bytes long unless the stream is eof.
bool upb_bytesrc_get(upb_bytesrc *src, upb_string *str, upb_strlen_t minlen);

// Appends the next "len" bytes in the stream in-place to "str".  This should
// be used when the caller needs to build a contiguous string of the existing
// data in "str" with more data.
bool upb_bytesrc_append(upb_bytesrc *src, upb_string *str, upb_strlen_t len);

// Returns the current error status for the stream.
INLINE upb_status *upb_bytesrc_status(upb_bytesrc *src) { return &src->status; }
INLINE bool upb_bytesrc_eof(upb_bytesrc *src) { return src->eof; }

/* upb_bytesink ***************************************************************/

// Puts the given string.  Returns the number of bytes that were actually,
// consumed, which may be fewer than were in the string, or <0 on error.
int32_t upb_bytesink_put(upb_bytesink *sink, upb_string *str);

// Returns the current error status for the stream.
upb_status *upb_bytesink_status(upb_bytesink *sink);

/* Utility functions **********************************************************/

// Streams data from src to sink until EOF or error.
void upb_streamdata(upb_src *src, upb_sink *sink, upb_status *status);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif