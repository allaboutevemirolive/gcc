/* AddressSanitizer, a fast memory error detector.
   Copyright (C) 2011-2016 Free Software Foundation, Inc.
   Contributed by Kostya Serebryany <kcc@google.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef TREE_ASAN
#define TREE_ASAN

extern void asan_function_start (void);
extern void asan_finish_file (void);
extern rtx_insn *asan_emit_stack_protection (rtx, rtx, unsigned int,
					     HOST_WIDE_INT *, tree *, int);
extern bool asan_protect_global (tree);
extern void initialize_sanitizer_builtins (void);
extern tree asan_dynamic_init_call (bool);
extern bool asan_expand_check_ifn (gimple_stmt_iterator *, bool);
extern bool asan_expand_mark_ifn (gimple_stmt_iterator *);

extern gimple_stmt_iterator create_cond_insert_point
     (gimple_stmt_iterator *, bool, bool, bool, basic_block *, basic_block *);

/* Alias set for accessing the shadow memory.  */
extern alias_set_type asan_shadow_set;

/* Hash set of labels that are either used in a goto, or their address
   has been taken.  */
extern hash_set <tree> *asan_used_labels;

/* Shadow memory is found at
   (address >> ASAN_SHADOW_SHIFT) + asan_shadow_offset ().  */
#define ASAN_SHADOW_SHIFT	3
#define ASAN_SHADOW_GRANULARITY (1UL << ASAN_SHADOW_SHIFT)

/* Red zone size, stack and global variables are padded by ASAN_RED_ZONE_SIZE
   up to 2 * ASAN_RED_ZONE_SIZE - 1 bytes.  */
#define ASAN_RED_ZONE_SIZE	32

/* Shadow memory values for stack protection.  Left is below protected vars,
   the first pointer in stack corresponding to that offset contains
   ASAN_STACK_FRAME_MAGIC word, the second pointer to a string describing
   the frame.  Middle is for padding in between variables, right is
   above the last protected variable and partial immediately after variables
   up to ASAN_RED_ZONE_SIZE alignment.  */
#define ASAN_STACK_MAGIC_LEFT		  0xf1
#define ASAN_STACK_MAGIC_MIDDLE		  0xf2
#define ASAN_STACK_MAGIC_RIGHT		  0xf3
#define ASAN_STACK_MAGIC_PARTIAL	  0xf4
#define ASAN_STACK_MAGIC_USE_AFTER_RET	  0xf5
#define ASAN_STACK_MAGIC_USE_AFTER_SCOPE  0xf8

#define ASAN_STACK_FRAME_MAGIC		0x41b58ab3
#define ASAN_STACK_RETIRED_MAGIC	0x45e0360e

/* Various flags for Asan builtins.  */
enum asan_check_flags
{
  ASAN_CHECK_STORE = 1 << 0,
  ASAN_CHECK_SCALAR_ACCESS = 1 << 1,
  ASAN_CHECK_NON_ZERO_LEN = 1 << 2,
  ASAN_CHECK_LAST = 1 << 3
};

/* Flags for Asan check builtins.  */
enum asan_mark_flags
{
  ASAN_MARK_CLOBBER = 1 << 0,
  ASAN_MARK_UNCLOBBER = 1 << 1,
  ASAN_MARK_LAST = 1 << 2
};

/* Return the size of padding needed to insert after a protected
   decl of SIZE.  */

static inline unsigned int
asan_red_zone_size (unsigned int size)
{
  unsigned int c = size & (ASAN_RED_ZONE_SIZE - 1);
  return c ? 2 * ASAN_RED_ZONE_SIZE - c : ASAN_RED_ZONE_SIZE;
}

extern bool set_asan_shadow_offset (const char *);

extern void set_sanitized_sections (const char *);

extern bool asan_sanitize_stack_p (void);

/* Return TRUE if builtin with given FCODE will be intercepted by
   libasan.  */

static inline bool
asan_intercepted_p (enum built_in_function fcode)
{
  return fcode == BUILT_IN_INDEX
	 || fcode == BUILT_IN_MEMCHR
	 || fcode == BUILT_IN_MEMCMP
	 || fcode == BUILT_IN_MEMCPY
	 || fcode == BUILT_IN_MEMMOVE
	 || fcode == BUILT_IN_MEMSET
	 || fcode == BUILT_IN_STRCASECMP
	 || fcode == BUILT_IN_STRCAT
	 || fcode == BUILT_IN_STRCHR
	 || fcode == BUILT_IN_STRCMP
	 || fcode == BUILT_IN_STRCPY
	 || fcode == BUILT_IN_STRDUP
	 || fcode == BUILT_IN_STRLEN
	 || fcode == BUILT_IN_STRNCASECMP
	 || fcode == BUILT_IN_STRNCAT
	 || fcode == BUILT_IN_STRNCMP
	 || fcode == BUILT_IN_STRNCPY;
}

/* Return TRUE if we should instrument for use-after-scope sanity checking.  */

static inline bool
asan_sanitize_use_after_scope (void)
{
  return (flag_sanitize_address_use_after_scope && asan_sanitize_stack_p ());
}

static inline bool
asan_no_sanitize_address_p (void)
{
  return lookup_attribute ("no_sanitize_address",
			   DECL_ATTRIBUTES (current_function_decl));
}

/* Return true if DECL should be guarded on the stack.  */

static inline bool
asan_protect_stack_decl (tree decl)
{
  return DECL_P (decl)
    && (!DECL_ARTIFICIAL (decl)
	|| (asan_sanitize_use_after_scope () && TREE_ADDRESSABLE (decl)));
}

#endif /* TREE_ASAN */
