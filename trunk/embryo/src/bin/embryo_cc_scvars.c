/*  Small compiler
 *
 *  Global (cross-module) variables.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id$
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <config.h>		/* for PATH_MAX */
#include "embryo_cc_sc.h"

/*  global variables
 *
 *  All global variables that are shared amongst the compiler files are
 *  declared here.
 */
symbol   loctab;	/* local symbol table */
symbol   glbtab;	/* global symbol table */
cell    *litq;	/* the literal queue */
char     pline[sLINEMAX + 1];	/* the line read from the input file */
char    *lptr;	/* points to the current position in "pline" */
constvalue tagname_tab = { NULL, "", 0, 0 };	/* tagname table */
constvalue libname_tab = { NULL, "", 0, 0 };	/* library table (#pragma library "..." syntax) */
constvalue *curlibrary = NULL;	/* current library */
symbol  *curfunc;	/* pointer to current function */
char    *inpfname;	/* pointer to name of the file currently read from */
char     outfname[PATH_MAX];	/* output file name */
char     sc_ctrlchar = CTRL_CHAR;	/* the control character (or escape character) */
int      litidx = 0;	/* index to literal table */
int      litmax = sDEF_LITMAX;	/* current size of the literal table */
int      stgidx = 0;	/* index to the staging buffer */
int      labnum = 0;	/* number of (internal) labels */
int      staging = 0;	/* true if staging output */
cell     declared = 0;	/* number of local cells declared */
cell     glb_declared = 0;	/* number of global cells declared */
cell     code_idx = 0;	/* number of bytes with generated code */
int      ntv_funcid = 0;	/* incremental number of native function */
int      errnum = 0;	/* number of errors */
int      warnnum = 0;	/* number of warnings */
int      sc_debug = sCHKBOUNDS;	/* by default: bounds checking+assertions */
int      charbits = 8;	/* a "char" is 8 bits */
int      sc_packstr = FALSE;	/* strings are packed by default? */
int      sc_compress = TRUE;	/* compress bytecode? */
int      sc_needsemicolon = TRUE;	/* semicolon required to terminate expressions? */
int      sc_dataalign = sizeof(cell);	/* data alignment value */
int      sc_alignnext = FALSE;	/* must frame of the next function be aligned? */
int      curseg = 0;	/* 1 if currently parsing CODE, 2 if parsing DATA */
cell     sc_stksize = sDEF_AMXSTACK;	/* default stack size */
int      freading = FALSE;	/* Is there an input file ready for reading? */
int      fline = 0;	/* the line number in the current file */
int      fnumber = 0;	/* the file number in the file table (debugging) */
int      fcurrent = 0;	/* current file being processed (debugging) */
int      intest = 0;	/* true if inside a test */
int      sideeffect = 0;	/* true if an expression causes a side-effect */
int      stmtindent = 0;	/* current indent of the statement */
int      indent_nowarn = TRUE;	/* skip warning "217 loose indentation" */
int      sc_tabsize = 8;	/* number of spaces that a TAB represents */
int      sc_allowtags = TRUE;	/* allow/detect tagnames in lex() */
int      sc_status;	/* read/write status */
int      sc_rationaltag = 0;	/* tag for rational numbers */
int      rational_digits = 0;	/* number of fractional digits */

FILE    *inpf = NULL;	/* file read from (source or include) */
FILE    *inpf_org = NULL;	/* main source file */
FILE    *outf = NULL;	/* file written to */

jmp_buf  errbuf;
