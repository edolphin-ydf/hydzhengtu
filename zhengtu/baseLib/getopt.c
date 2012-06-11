#include <baseLib/platForm.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
//#include <string.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else //HAVE_GETOPT_H
#include <baseLib/getopt.h>
#endif //HAVE_GETOPT_H

#ifndef HAVE_ARGP_H
#include <baseLib/argp.h>
#include <baseLib/strLib.h>

#ifndef _SSIZE_T_
#define _SSIZE_T_
typedef int ssize_t;
#endif //_SSIZE_T_

#ifdef WIN32
typedef unsigned char uint8_t;
#endif //WIN32

int _getopt_internal(int ___argc, char *const *___argv,
			     const char *__shortopts,
		             const struct option *__longopts, int *__longind,
			     int __long_only);

/* Reentrant versions which can handle parsing multiple argument
   vectors at the same time.  */

/* Data type for reentrant functions.  */
struct _getopt_data
{
  /* These have exactly the same meaning as the corresponding global
     variables, except that they are used for the reentrant
     versions of getopt.  */
  int optind;
  int opterr;
  int optopt;
  char *optarg;

  /* Internal members.  */

  /* True if the internal members have been initialized.  */
  int __initialized;

  /* The next char to be scanned in the option-element
     in which the last option character we returned was found.
     This allows us to pick up the scan where we left off.

     If this is zero, or a null string, it means resume the scan
     by advancing to the next ARGV-element.  */
  char *__nextchar;

  /* Describe how to deal with options that follow non-option ARGV-elements.

     If the caller did not specify anything,
     the default is REQUIRE_ORDER if the environment variable
     POSIXLY_CORRECT is defined, PERMUTE otherwise.

     REQUIRE_ORDER means don't recognize them as options;
     stop option processing when the first non-option is seen.
     This is what Unix does.
     This mode of operation is selected by either setting the environment
     variable POSIXLY_CORRECT, or using `+' as the first character
     of the list of option characters.

     PERMUTE is the default.  We permute the contents of ARGV as we
     scan, so that eventually all the non-options are at the end.
     This allows options to be given in any order, even with programs
     that were not written to expect this.

     RETURN_IN_ORDER is an option available to programs that were
     written to expect options and other ARGV-elements in any order
     and that care about the ordering of the two.  We describe each
     non-option ARGV-element as if it were the argument of an option
     with character code 1.  Using `-' as the first character of the
     list of option characters selects this mode of operation.

     The special argument `--' forces an end of option-scanning regardless
     of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
     `--' can cause `getopt' to return -1 with `optind' != ARGC.  */

  enum
    {
      REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
    } __ordering;

  /* If the POSIXLY_CORRECT environment variable is set.  */
  int __posixly_correct;


  /* Handle permutation of arguments.  */

  /* Describe the part of ARGV that contains non-options that have
     been skipped.  `first_nonopt' is the index in ARGV of the first
     of them; `last_nonopt' is the index after the last of them.  */

  int __first_nonopt;
  int __last_nonopt;
};

/* The initializer is necessary to set OPTIND and OPTERR to their
   default values and to clear the initialization flag.  */
#define _GETOPT_DATA_INITIALIZER	{ 1, 1 }

int _getopt_internal_r (int ___argc, char *const *___argv,
			       const char *__shortopts,
			       const struct option *__longopts, int *__longind,
			       int __long_only, struct _getopt_data *__data);

int _getopt_long_r (int ___argc, char *const *___argv,
			   const char *__shortopts,
			   const struct option *__longopts, int *__longind,
			   struct _getopt_data *__data);

int _getopt_long_only_r (int ___argc, char *const *___argv,
				const char *__shortopts,
				const struct option *__longopts,
				int *__longind,
				struct _getopt_data *__data);


struct argp_fmtstream
{
  FILE *stream;			/* The stream we're outputting to.  */

  size_t lmargin, rmargin;	/* Left and right margins.  */
  ssize_t wmargin;		/* Margin to wrap to, or -1 to truncate.  */

  /* Point in buffer to which we've processed for wrapping, but not output.  */
  size_t point_offs;
  /* Output column at POINT_OFFS, or -1 meaning 0 but don't add lmargin.  */
  ssize_t point_col;

  char *buf;			/* Output buffer.  */
  char *p;			/* Current end of text in BUF. */
  char *end;			/* Absolute end of BUF.  */
};

typedef struct argp_fmtstream *argp_fmtstream_t;

/* Return an argp_fmtstream that outputs to STREAM, and which prefixes lines
   written on it with LMARGIN spaces and limits them to RMARGIN columns
   total.  If WMARGIN >= 0, words that extend past RMARGIN are wrapped by
   replacing the whitespace before them with a newline and WMARGIN spaces.
   Otherwise, chars beyond RMARGIN are simply dropped until a newline.
   Returns NULL if there was an error.  */
argp_fmtstream_t argp_make_fmtstream (FILE *__stream,
					       size_t __lmargin,
					       size_t __rmargin,
					       ssize_t __wmargin);
argp_fmtstream_t argp_make_fmtstream (FILE *__stream,
					     size_t __lmargin,
					     size_t __rmargin,
					     ssize_t __wmargin);

/* Flush __FS to its stream, and free it (but don't close the stream).  */
void argp_fmtstream_free (argp_fmtstream_t __fs);
void argp_fmtstream_free (argp_fmtstream_t __fs);

ssize_t argp_fmtstream_printf (argp_fmtstream_t __fs,
				       const char *__fmt, ...);
ssize_t argp_fmtstream_printf (argp_fmtstream_t __fs,
				      const char *__fmt, ...);

int argp_fmtstream_putc (argp_fmtstream_t __fs, int __ch);
int argp_fmtstream_putc (argp_fmtstream_t __fs, int __ch);

int argp_fmtstream_puts (argp_fmtstream_t __fs, const char *__str);
int argp_fmtstream_puts (argp_fmtstream_t __fs, const char *__str);

size_t argp_fmtstream_write (argp_fmtstream_t __fs,
				      const char *__str, size_t __len);
size_t argp_fmtstream_write (argp_fmtstream_t __fs,
				    const char *__str, size_t __len);

/* Access macros for various bits of state.  */
#define argp_fmtstream_lmargin(__fs) ((__fs)->lmargin)
#define argp_fmtstream_rmargin(__fs) ((__fs)->rmargin)
#define argp_fmtstream_wmargin(__fs) ((__fs)->wmargin)

/* Set __FS's left margin to LMARGIN and return the old value.  */
size_t argp_fmtstream_set_lmargin (argp_fmtstream_t __fs,
					  size_t __lmargin);
size_t argp_fmtstream_set_lmargin (argp_fmtstream_t __fs,
					    size_t __lmargin);

/* Set __FS's right margin to __RMARGIN and return the old value.  */
size_t argp_fmtstream_set_rmargin (argp_fmtstream_t __fs,
					  size_t __rmargin);
size_t argp_fmtstream_set_rmargin (argp_fmtstream_t __fs,
					    size_t __rmargin);

/* Set __FS's wrap margin to __WMARGIN and return the old value.  */
size_t argp_fmtstream_set_wmargin (argp_fmtstream_t __fs,
					  size_t __wmargin);
size_t argp_fmtstream_set_wmargin (argp_fmtstream_t __fs,
					    size_t __wmargin);

/* Return the column number of the current output point in __FS.  */
size_t argp_fmtstream_point (argp_fmtstream_t __fs);
size_t argp_fmtstream_point (argp_fmtstream_t __fs);

/* Internal routines.  */
void _argp_fmtstream_update (argp_fmtstream_t __fs);
void argp_fmtstream_update (argp_fmtstream_t __fs);
int _argp_fmtstream_ensure (argp_fmtstream_t __fs, size_t __amount);
int argp_fmtstream_ensure (argp_fmtstream_t __fs, size_t __amount);

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also,when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */
#endif //HAVE_ARGP_H

#ifndef HAVE_GETOPT_H

char *optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt',zero means this is the first call; initialize.

   When `getopt' returns -1,this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise,`optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

/* 1003.2 says this must be 1 before any call.  */
int optind = 1;

/* Callers store zero here to inhibit the error message
   for unrecognized options.  */

int opterr = 1;

/* Set to an option character which was unrecognized.
   This must be initialized on some systems to avoid linking in the
   system's own getopt implementation.  */

int optopt = '?';

/* Keep a global copy of all internal members of getopt_data.  */

static struct _getopt_data getopt_data;
#endif //HAVE_GETOPT_H

#ifndef HAVE_ARGP_H

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind),which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */

static void
exchange(char **argv,struct _getopt_data *d)
{
  int bottom = d->__first_nonopt;
  int middle = d->__last_nonopt;
  int top = d->optind;
  char *tem;

  /* Exchange the shorter segment with the far end of the longer segment.
     That puts the shorter segment into the right place.
     It leaves the longer segment in the right place overall,
     but it consists of two parts that need to be swapped next.  */

  while(top > middle && middle > bottom)
    {
      if(top - middle > middle - bottom)
	{
	  /* Bottom segment is the short one.  */
	  int len = middle - bottom;
	  register int i;

	  /* Swap it with the top part of the top segment.  */
	  for(i = 0; i < len; i++)
	    {
	      tem = argv[bottom + i];
	      argv[bottom + i] = argv[top -(middle - bottom) + i];
	      argv[top -(middle - bottom) + i] = tem;
	    }
	  /* Exclude the moved bottom segment from further swapping.  */
	  top -= len;
	}
      else
	{
	  /* Top segment is the short one.  */
	  int len = top - middle;
	  register int i;

	  /* Swap it with the bottom part of the bottom segment.  */
	  for(i = 0; i < len; i++)
	    {
	      tem = argv[bottom + i];
	      argv[bottom + i] = argv[middle + i];
	      argv[middle + i] = tem;
	    }
	  /* Exclude the moved top segment from further swapping.  */
	  bottom += len;
	}
    }

  /* Update records for the slots the non-options now occupy.  */

  d->__first_nonopt +=(d->optind - d->__last_nonopt);
  d->__last_nonopt = d->optind;
}

/* Initialize the internal data when the first call is made.  */

static const char *
_getopt_initialize(int argc,char *const *argv,const char *optstring,
		    struct _getopt_data *d)
{
  /* Start processing options with ARGV-element 1(since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  d->__first_nonopt = d->__last_nonopt = d->optind;

  d->__nextchar = NULL;

  d->__posixly_correct = !!getenv("POSIXLY_CORRECT");

  /* Determine how to handle the ordering of options and nonoptions.  */

  if(optstring[0] == '-')
    {
      d->__ordering = RETURN_IN_ORDER;
      ++optstring;
    }
  else if(optstring[0] == '+')
    {
      d->__ordering = REQUIRE_ORDER;
      ++optstring;
    }
  else if(d->__posixly_correct)
    d->__ordering = REQUIRE_ORDER;
  else
    d->__ordering = PERMUTE;

  return optstring;
}

/* Scan elements of ARGV(whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-',and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
(aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly,it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character,it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters,`getopt' returns -1.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.(The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return '?' after printing an error message.  If you set `opterr' to
   zero,the error message is suppressed but we still return '?'.

   If a char in OPTSTRING is followed by a colon,that means it wants an arg,
   so the following text in the same ARGV-element,or the text of the following
   ARGV-element,is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg',otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+',it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER,above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument,it follows the option name in the same ARGV-element,separated
   from the option name by a `=',or else the in next ARGV-element.
   When `getopt' finds a long-named option,it returns 0 if that option's
   `flag' field is nonzero,the value of the option's `val' field
   if the `flag' field is zero.

   The elements of ARGV aren't really const,because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero,'-' as well as '--' can introduce
   long-named options.  */

int
_getopt_internal_r(int argc,char *const *argv,const char *optstring,
		    const struct option *longopts,int *longind,
		    int long_only,struct _getopt_data *d)
{
  int print_errors = d->opterr;
  if(optstring[0] == ':')
    print_errors = 0;

  if(argc < 1)
    return -1;

  d->optarg = NULL;

  if(d->optind == 0 || !d->__initialized)
    {
      if(d->optind == 0)
	d->optind = 1;	/* Don't scan ARGV[0],the program name.  */
      optstring = _getopt_initialize(argc,argv,optstring,d);
      d->__initialized = 1;
    }

  /* Test whether ARGV[optind] points to a non-option argument.
     Either it does not have option syntax,or there is an environment flag
     from the shell indicating it is not an option.  The later information
     is only used when the used in the GNU libc.  */
# define NONOPTION_P ((argv[d->optind][0] != '-' && argv[d->optind][0] != '/') || argv[d->optind][1] == '\0')

  if(d->__nextchar == NULL || *d->__nextchar == '\0')
    {
      /* Advance to the next ARGV-element.  */

      /* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
	 moved back by the user(who may also have changed the arguments).  */
      if(d->__last_nonopt > d->optind)
	d->__last_nonopt = d->optind;
      if(d->__first_nonopt > d->optind)
	d->__first_nonopt = d->optind;

      if(d->__ordering == PERMUTE)
	{
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

	  if(d->__first_nonopt != d->__last_nonopt
	      && d->__last_nonopt != d->optind)
	    exchange((char **) argv,d);
	  else if(d->__last_nonopt != d->optind)
	    d->__first_nonopt = d->optind;

	  /* Skip any additional non-options
	     and extend the range of non-options previously skipped.  */

	  while(d->optind < argc && NONOPTION_P)
	    d->optind++;
	  d->__last_nonopt = d->optind;
	}

      /* The special ARGV-element `--' means premature end of options.
	 Skip it like a null option,
	 then exchange with previous non-options as if it were an option,
	 then skip everything else like a non-option.  */

      if(d->optind != argc && !strcmp(argv[d->optind],"--"))
	{
	  d->optind++;

	  if(d->__first_nonopt != d->__last_nonopt
	      && d->__last_nonopt != d->optind)
	    exchange((char **) argv,d);
	  else if(d->__first_nonopt == d->__last_nonopt)
	    d->__first_nonopt = d->optind;
	  d->__last_nonopt = argc;

	  d->optind = argc;
	}

      /* If we have done all the ARGV-elements,stop the scan
	 and back over any non-options that we skipped and permuted.  */

      if(d->optind == argc)
	{
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped,so the caller will digest them.  */
	  if(d->__first_nonopt != d->__last_nonopt)
	    d->optind = d->__first_nonopt;
	  return -1;
	}

      /* If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

      if(NONOPTION_P)
	{
	  if(d->__ordering == REQUIRE_ORDER)
	    return -1;
	  d->optarg = argv[d->optind++];
	  return 1;
	}

      /* We have found another option-ARGV-element.
	 Skip the initial punctuation.  */

      d->__nextchar =(argv[d->optind] + 1
		  +(longopts != NULL && argv[d->optind][1] == '-'));
    }

  /* Decode the current option-ARGV-element.  */

  /* Check whether the ARGV-element is a long option.

     If long_only and the ARGV-element has the form "-f",where f is
     a valid short option,don't consider it an abbreviated form of
     a long option that starts with f.  Otherwise there would be no
     way to give the -f short option.

     On the other hand,if there's a long option "fubar" and
     the ARGV-element is "-fu",do consider that an abbreviation of
     the long option,just like "--fu",and not "-f" with arg "u".

     This distinction seems to be the most useful approach.  */

  if(longopts != NULL
      &&(argv[d->optind][1] == '-'
	  ||(long_only &&(argv[d->optind][2]
			    || !strchr(optstring,argv[d->optind][1])))))
    {
      char *nameend;
      const struct option *p;
      const struct option *pfound = NULL;
      int exact = 0;
      int ambig = 0;
      int indfound = -1;
      int option_index;

      for(nameend = d->__nextchar; *nameend && *nameend != '='; nameend++)
	/* Do nothing.  */ ;

      /* Test all long options for either exact match
	 or abbreviated matches.  */
      for(p = longopts,option_index = 0; p->name; p++,option_index++)
	if(!strncmp(p->name,d->__nextchar,nameend - d->__nextchar))
	  {
	    if((unsigned int)(nameend - d->__nextchar)
		==(unsigned int) strlen(p->name))
	      {
		/* Exact match found.  */
		pfound = p;
		indfound = option_index;
		exact = 1;
		break;
	      }
	    else if(pfound == NULL)
	      {
		/* First nonexact match found.  */
		pfound = p;
		indfound = option_index;
	      }
	    else if(long_only
		     || pfound->has_arg != p->has_arg
		     || pfound->flag != p->flag
		     || pfound->val != p->val)
	      /* Second or later nonexact match found.  */
	      ambig = 1;
	  }

      if(ambig && !exact)
	{
	  if(print_errors)
	    {
	      fprintf(stderr,"%s: option `%s' is ambiguous\n",
		       argv[0],argv[d->optind]);
	    }
	  d->__nextchar += strlen(d->__nextchar);
	  d->optind++;
	  d->optopt = 0;
	  return '?';
	}

      if(pfound != NULL)
	{
	  option_index = indfound;
	  d->optind++;
	  if(*nameend)
	    {
	      /* Don't test has_arg with >,because some C compilers don't
		 allow it to be used on enums.  */
	      if(pfound->has_arg)
		d->optarg = nameend + 1;
	      else
		{
		  if(print_errors)
		    {
		      if(argv[d->optind - 1][1] == '-')
			{
			  /* --option */
			  fprintf(stderr,"%s: option `--%s' doesn't allow an argument\n",
				   argv[0],pfound->name);
			}
		      else
			{
			  /* +option or -option */
			  fprintf(stderr,"%s: option `%c%s' doesn't allow an argument\n",
				   argv[0],argv[d->optind - 1][0],
				   pfound->name);
			}
		    }

		  d->__nextchar += strlen(d->__nextchar);

		  d->optopt = pfound->val;
		  return '?';
		}
	    }
	  else if(pfound->has_arg == 1)
	    {
	      if(d->optind < argc)
		d->optarg = argv[d->optind++];
	      else
		{
		  if(print_errors)
		    {
		      fprintf(stderr,
			       "%s: option `%s' requires an argument\n",
			       argv[0],argv[d->optind - 1]);
		    }
		  d->__nextchar += strlen(d->__nextchar);
		  d->optopt = pfound->val;
		  return optstring[0] == ':' ? ':' : '?';
		}
	    }
	  d->__nextchar += strlen(d->__nextchar);
	  if(longind != NULL)
	    *longind = option_index;
	  if(pfound->flag)
	    {
	      *(pfound->flag) = pfound->val;
	      return 0;
	    }
	  return pfound->val;
	}

      /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option,then it's an error.
	 Otherwise interpret it as a short option.  */
      if(!long_only || argv[d->optind][1] == '-'
	  || strchr(optstring,*d->__nextchar) == NULL)
	{
	  if(print_errors)
	    {
	      if(argv[d->optind][1] == '-')
		{
		  /* --option */
		  fprintf(stderr,"%s: unrecognized option `--%s'\n",
			   argv[0],d->__nextchar);
		}
	      else
		{
		  /* +option or -option */
	  fprintf(stderr,"%s: unrecognized option `%c%s'\n",
			   argv[0],argv[d->optind][0],d->__nextchar);
		}
	    }
	  d->__nextchar =(char *) "";
	  d->optind++;
	  d->optopt = 0;
	  return '?';
	}
    }

  /* Look at and handle the next short option-character.  */

  {
    char c = *d->__nextchar++;
    char *temp = strchr(optstring,c);

    /* Increment `optind' when we start to process its last character.  */
    if(*d->__nextchar == '\0')
      ++d->optind;

    if(temp == NULL || c == ':')
      {
	if(print_errors)
	  {
	    if(d->__posixly_correct)
	      {
		/* 1003.2 specifies the format of this message.  */
		fprintf(stderr,"%s: illegal option -- %c\n",argv[0],c);
	      }
	    else
	      {
fprintf(stderr,"%s: invalid option -- %c\n",argv[0],c);
	      }
	  }
	d->optopt = c;
	return '?';
      }
    /* Convenience. Treat POSIX -W foo same as long option --foo */
    if(temp[0] == 'W' && temp[1] == ';')
      {
	char *nameend;
	const struct option *p;
	const struct option *pfound = NULL;
	int exact = 0;
	int ambig = 0;
	int indfound = 0;
	int option_index;

	/* This is an option that requires an argument.  */
	if(*d->__nextchar != '\0')
	  {
	    d->optarg = d->__nextchar;
	    /* If we end this ARGV-element by taking the rest as an arg,
	       we must advance to the next element now.  */
	    d->optind++;
	  }
	else if(d->optind == argc)
	  {
	    if(print_errors)
	      {
		/* 1003.2 specifies the format of this message.  */
		fprintf(stderr,"%s: option requires an argument -- %c\n",
			 argv[0],c);
	      }
	    d->optopt = c;
	    if(optstring[0] == ':')
	      c = ':';
	    else
	      c = '?';
	    return c;
	  }
	else
	  /* We already incremented `d->optind' once;
	     increment it again when taking next ARGV-elt as argument.  */
	  d->optarg = argv[d->optind++];

	/* optarg is now the argument,see if it's in the
	   table of longopts.  */

	for(d->__nextchar = nameend = d->optarg; *nameend && *nameend != '=';
	     nameend++)
	  /* Do nothing.  */ ;

	/* Test all long options for either exact match
	   or abbreviated matches.  */
	for(p = longopts,option_index = 0; p->name; p++,option_index++)
	  if(!strncmp(p->name,d->__nextchar,nameend - d->__nextchar))
	    {
	      if((unsigned int)(nameend - d->__nextchar) == strlen(p->name))
		{
		  /* Exact match found.  */
		  pfound = p;
		  indfound = option_index;
		  exact = 1;
		  break;
		}
	      else if(pfound == NULL)
		{
		  /* First nonexact match found.  */
		  pfound = p;
		  indfound = option_index;
		}
	      else
		/* Second or later nonexact match found.  */
		ambig = 1;
	    }
	if(ambig && !exact)
	  {
	    if(print_errors)
	      {
		fprintf(stderr,"%s: option `-W %s' is ambiguous\n",
			 argv[0],argv[d->optind]);
	      }
	    d->__nextchar += strlen(d->__nextchar);
	    d->optind++;
	    return '?';
	  }
	if(pfound != NULL)
	  {
	    option_index = indfound;
	    if(*nameend)
	      {
		/* Don't test has_arg with >,because some C compilers don't
		   allow it to be used on enums.  */
		if(pfound->has_arg)
		  d->optarg = nameend + 1;
		else
		  {
		    if(print_errors)
		      {
			fprintf(stderr,"%s: option `-W %s' doesn't allow an argument\n",
				 argv[0],pfound->name);
		      }

		    d->__nextchar += strlen(d->__nextchar);
		    return '?';
		  }
	      }
	    else if(pfound->has_arg == 1)
	      {
		if(d->optind < argc)
		  d->optarg = argv[d->optind++];
		else
		  {
		    if(print_errors)
		      {
			fprintf(stderr,
				 "%s: option `%s' requires an argument\n",
				 argv[0],argv[d->optind - 1]);
		      }
		    d->__nextchar += strlen(d->__nextchar);
		    return optstring[0] == ':' ? ':' : '?';
		  }
	      }
	    d->__nextchar += strlen(d->__nextchar);
	    if(longind != NULL)
	      *longind = option_index;
	    if(pfound->flag)
	      {
		*(pfound->flag) = pfound->val;
		return 0;
	      }
	    return pfound->val;
	  }
	  d->__nextchar = NULL;
	  return 'W';	/* Let the application handle it.   */
      }
    if(temp[1] == ':')
      {
	if(temp[2] == ':')
	  {
	    /* This is an option that accepts an argument optionally.  */
	    if(*d->__nextchar != '\0')
	      {
		d->optarg = d->__nextchar;
		d->optind++;
	      }
	    else
	      d->optarg = NULL;
	    d->__nextchar = NULL;
	  }
	else
	  {
	    /* This is an option that requires an argument.  */
	    if(*d->__nextchar != '\0')
	      {
		d->optarg = d->__nextchar;
		/* If we end this ARGV-element by taking the rest as an arg,
		   we must advance to the next element now.  */
		d->optind++;
	      }
	    else if(d->optind == argc)
	      {
		if(print_errors)
		  {
		    /* 1003.2 specifies the format of this message.  */
		    fprintf(stderr,
			     "%s: option requires an argument -- %c\n",
			     argv[0],c);
		  }
		d->optopt = c;
		if(optstring[0] == ':')
		  c = ':';
		else
		  c = '?';
	      }
	    else
	      /* We already incremented `optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
	      d->optarg = argv[d->optind++];
	    d->__nextchar = NULL;
	  }
      }
    return c;
  }
}

int
_getopt_long_r (int argc, char *const *argv, const char *options,
		const struct option *long_options, int *opt_index,
		struct _getopt_data *d)
{
  return _getopt_internal_r (argc, argv, options, long_options, opt_index,
			     0, d);
}

int
_getopt_long_only_r (int argc, char *const *argv, const char *options,
		     const struct option *long_options, int *opt_index,
		     struct _getopt_data *d)
{
  return _getopt_internal_r (argc, argv, options, long_options, opt_index,
			     1, d);
}
#endif //HAVE_ARGP_H

#ifndef HAVE_GETOPT_H
int _getopt_internal(int argc,char *const *argv,const char *optstring,const struct option *longopts,int *longind,int long_only)
{
  int result;

  getopt_data.optind = optind;
  getopt_data.opterr = opterr;

  result = _getopt_internal_r(argc,argv,optstring,longopts,
			       longind,long_only,&getopt_data);

  optind = getopt_data.optind;
  optarg = getopt_data.optarg;
  optopt = getopt_data.optopt;

  return result;
}

int getopt(int argc,char *const *argv,const char *optstring)
{
  return _getopt_internal(argc,argv,optstring,NULL,NULL,0);
}

int
getopt_long (int argc, char *const *argv, const char *options,
	     const struct option *long_options, int *opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

/* Like getopt_long, but '-' as well as '--' can indicate a long option.
   If an option that starts with '-' (not '--') doesn't match a long option,
   but does match a short option, it is parsed as a short option
   instead.  */

int
getopt_long_only (int argc, char *const *argv, const char *options,
		  const struct option *long_options, int *opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 1);
}
#endif //HAVE_GETOPT_H

#ifndef HAVE_ARGP_H

#define EX_USAGE 0

/* The exit status that argp will use when exiting due to a parsing error.
   If not defined or set by the user program, this defaults to EX_USAGE from
   <sysexits.h>.  */
error_t argp_err_exit_status = EX_USAGE;

/* If set by the user program to a non-zero value, then a default option
   --version is added (unless the ARGP_NO_HELP flag is used), which will
   print this this string followed by a newline and exit (unless the
   ARGP_NO_EXIT flag is used).  Overridden by ARGP_PROGRAM_VERSION_HOOK.  */
const char *argp_program_version;

const char *argp_program_bug_address;

/* If set by the user program to a non-zero value, then a default option
   --version is added (unless the ARGP_NO_HELP flag is used), which calls
   this function with a stream to print the version to and a pointer to the
   current parsing state, and then exits (unless the ARGP_NO_EXIT flag is
   used).  This variable takes precedent over ARGP_PROGRAM_VERSION.  */
void (*argp_program_version_hook) (FILE *stream, struct argp_state *state);

/* Getopt return values.  */
#define KEY_END (-1)		/* The end of the options.  */
#define KEY_ARG 1		/* A non-option argument.  */
#define KEY_ERR '?'		/* An error parsing the options.  */

/* The meta-argument used to prevent any further arguments being interpreted
   as options.  */
#define QUOTE "--"

/* The number of bits we steal in a long-option value for our own use.  */
#define GROUP_BITS CHAR_BIT

/* The number of bits available for the user value.  */
#define USER_BITS ((sizeof ((struct option *)0)->val * CHAR_BIT) - GROUP_BITS)
#define USER_MASK ((1 << USER_BITS) - 1)

/* EZ alias for ARGP_ERR_UNKNOWN.  */
#define EBADKEY ARGP_ERR_UNKNOWN

/* Default options.  */

/* When argp is given the --HANG switch, _ARGP_HANG is set and argp will sleep
   for one second intervals, decrementing _ARGP_HANG until it's zero.  Thus
   you can force the program to continue by attaching a debugger and setting
   it to 0 yourself.  */
static volatile int _argp_hang;

#define OPT_PROGNAME	-2
#define OPT_USAGE	-3
#define OPT_HANG	-4

static const struct argp_option argp_default_options[] =
{
  {"help",	  '?',    	0, 0,  "Give this help list", -1},
  {"usage",	  OPT_USAGE,	0, 0,  "Give a short usage message"},
  {"program-name",OPT_PROGNAME,"NAME", OPTION_HIDDEN, "Set the program name"},
  {"HANG",	  OPT_HANG,    "SECS", OPTION_ARG_OPTIONAL | OPTION_HIDDEN,
     "Hang for SECS seconds (default 3600)"},
  {0, 0}
};

static error_t
argp_default_parser (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case '?':
      argp_state_help (state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case OPT_USAGE:
      argp_state_help (state, state->out_stream,
		       ARGP_HELP_USAGE | ARGP_HELP_EXIT_OK);
      break;

    case OPT_PROGNAME:		/* Set the program name.  */
      /* [Note that some systems only have PROGRAM_INVOCATION_SHORT_NAME (aka
	 __PROGNAME), in which case, PROGRAM_INVOCATION_NAME is just defined
	 to be that, so we have to be a bit careful here.]  */

      /* Update what we use for messages.  */
      state->name = strrchr (arg, '/');
      if (state->name)
	state->name++;
      else
	state->name = arg;

      if ((state->flags & (ARGP_PARSE_ARGV0 | ARGP_NO_ERRS))
	  == ARGP_PARSE_ARGV0)
	/* Update what getopt uses too.  */
	state->argv[0] = arg;

      break;

    case OPT_HANG:
      _argp_hang = atoi (arg ? arg : "3600");
      while (_argp_hang-- > 0)
#ifdef WIN32
	Sleep(1000);
#else //WIN32
    sleep(1);
#endif //WIN32
      break;

    default:
      return EBADKEY;
    }
  return 0;
}

static const struct argp argp_default_argp =
  {argp_default_options, &argp_default_parser, NULL, NULL, NULL, NULL, "libc"};


static const struct argp_option argp_version_options[] =
{
  {"version",	  'V',    	0, 0,  "Print program version", -1},
  {0, 0}
};

static error_t
argp_version_parser (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'V':
      if (argp_program_version_hook)
	(*argp_program_version_hook) (state->out_stream, state);
      else if (argp_program_version)
	fprintf (state->out_stream, "%s\n", argp_program_version);
      else
	argp_error (state, "PROGRAM ERROR) No version known!?");
      if (! (state->flags & ARGP_NO_EXIT))
	exit (0);
      break;
    default:
      return EBADKEY;
    }
  return 0;
}

static const struct argp argp_version_argp =
  {argp_version_options, &argp_version_parser, NULL, NULL, NULL, NULL, "libc"};

/* Returns the offset into the getopt long options array LONG_OPTIONS of a
   long option with called NAME, or -1 if none is found.  Passing NULL as
   NAME will return the number of options.  */
static int
find_long_option (struct option *long_options, const char *name)
{
  struct option *l = long_options;
  while (l->name != NULL)
    if (name != NULL && strcmp (l->name, name) == 0)
      return l - long_options;
    else
      l++;
  if (name == NULL)
    return l - long_options;
  else
    return -1;
}


/* The state of a `group' during parsing.  Each group corresponds to a
   particular argp structure from the tree of such descending from the top
   level argp passed to argp_parse.  */
struct group
{
  /* This group's parsing function.  */
  argp_parser_t parser;

  /* Which argp this group is from.  */
  const struct argp *argp;

  /* Points to the point in SHORT_OPTS corresponding to the end of the short
     options for this group.  We use it to determine from which group a
     particular short options is from.  */
  char *short_end;

  /* The number of non-option args sucessfully handled by this parser.  */
  unsigned args_processed;

  /* This group's parser's parent's group.  */
  struct group *parent;
  unsigned parent_index;	/* And the our position in the parent.   */

  /* These fields are swapped into and out of the state structure when
     calling this group's parser.  */
  void *input, **child_inputs;
  void *hook;
};

/* Call GROUP's parser with KEY and ARG, swapping any group-specific info
   from STATE before calling, and back into state afterwards.  If GROUP has
   no parser, EBADKEY is returned.  */
static error_t
group_parse (struct group *group, struct argp_state *state, int key, char *arg)
{
  if (group->parser)
    {
      error_t err;
      state->hook = group->hook;
      state->input = group->input;
      state->child_inputs = group->child_inputs;
      state->arg_num = group->args_processed;
      err = (*group->parser)(key, arg, state);
      group->hook = state->hook;
      return err;
    }
  else
    return EBADKEY;
}

struct parser
{
  const struct argp *argp;

  /* SHORT_OPTS is the getopt short options string for the union of all the
     groups of options.  */
  char *short_opts;
  /* LONG_OPTS is the array of getop long option structures for the union of
     all the groups of options.  */
  struct option *long_opts;
  /* OPT_DATA is the getopt data used for the re-entrant getopt.  */
  struct _getopt_data opt_data;

  /* States of the various parsing groups.  */
  struct group *groups;
  /* The end of the GROUPS array.  */
  struct group *egroup;
  /* An vector containing storage for the CHILD_INPUTS field in all groups.  */
  void **child_inputs;

  /* True if we think using getopt is still useful; if false, then
     remaining arguments are just passed verbatim with ARGP_KEY_ARG.  This is
     cleared whenever getopt returns KEY_END, but may be set again if the user
     moves the next argument pointer backwards.  */
  int try_getopt;

  /* State block supplied to parsing routines.  */
  struct argp_state state;

  /* Memory used by this parser.  */
  void *storage;
};

/* The next usable entries in the various parser tables being filled in by
   convert_options.  */
struct parser_convert_state
{
  struct parser *parser;
  char *short_end;
  struct option *long_end;
  void **child_inputs_end;
};

/* Converts all options in ARGP (which is put in GROUP) and ancestors
   into getopt options stored in SHORT_OPTS and LONG_OPTS; SHORT_END and
   CVT->LONG_END are the points at which new options are added.  Returns the
   next unused group entry.  CVT holds state used during the conversion.  */
static struct group *
convert_options (const struct argp *argp,
		 struct group *parent, unsigned parent_index,
		 struct group *group, struct parser_convert_state *cvt)
{
  /* REAL is the most recent non-alias value of OPT.  */
  const struct argp_option *real = argp->options;
  const struct argp_child *children = argp->children;

  if (real || argp->parser)
    {
      const struct argp_option *opt;

      if (real)
	for (opt = real; !_option_is_end (opt); opt++)
	  {
	    if (! (opt->flags & OPTION_ALIAS))
	      /* OPT isn't an alias, so we can use values from it.  */
	      real = opt;

	    if (! (real->flags & OPTION_DOC))
	      /* A real option (not just documentation).  */
	      {
		if (_option_is_short (opt))
		  /* OPT can be used as a short option.  */
		  {
		    *cvt->short_end++ = opt->key;
		    if (real->arg)
		      {
			*cvt->short_end++ = ':';
			if (real->flags & OPTION_ARG_OPTIONAL)
			  *cvt->short_end++ = ':';
		      }
		    *cvt->short_end = '\0'; /* keep 0 terminated */
		  }

		if (opt->name
		    && find_long_option (cvt->parser->long_opts, opt->name) < 0)
		  /* OPT can be used as a long option.  */
		  {
		    cvt->long_end->name = opt->name;
		    cvt->long_end->has_arg =
		      (real->arg
		       ? (real->flags & OPTION_ARG_OPTIONAL
			  ? optional_argument
			  : required_argument)
		       : no_argument);
		    cvt->long_end->flag = 0;
		    /* we add a disambiguating code to all the user's
		       values (which is removed before we actually call
		       the function to parse the value); this means that
		       the user loses use of the high 8 bits in all his
		       values (the sign of the lower bits is preserved
		       however)...  */
		    cvt->long_end->val =
		      ((opt->key | real->key) & USER_MASK)
		      + (((group - cvt->parser->groups) + 1) << USER_BITS);

		    /* Keep the LONG_OPTS list terminated.  */
		    (++cvt->long_end)->name = NULL;
		  }
	      }
	    }

      group->parser = argp->parser;
      group->argp = argp;
      group->short_end = cvt->short_end;
      group->args_processed = 0;
      group->parent = parent;
      group->parent_index = parent_index;
      group->input = 0;
      group->hook = 0;
      group->child_inputs = 0;

      if (children)
	/* Assign GROUP's CHILD_INPUTS field some space from
           CVT->child_inputs_end.*/
	{
	  unsigned num_children = 0;
	  while (children[num_children].argp)
	    num_children++;
	  group->child_inputs = cvt->child_inputs_end;
	  cvt->child_inputs_end += num_children;
	}

      parent = group++;
    }
  else
    parent = 0;

  if (children)
    {
      unsigned index = 0;
      while (children->argp)
	group =
	  convert_options (children++->argp, parent, index++, group, cvt);
    }

  return group;
}

/* Find the merged set of getopt options, with keys appropiately prefixed. */
static void
parser_convert (struct parser *parser, const struct argp *argp, int flags)
{
  struct parser_convert_state cvt;

  cvt.parser = parser;
  cvt.short_end = parser->short_opts;
  cvt.long_end = parser->long_opts;
  cvt.child_inputs_end = parser->child_inputs;

  if (flags & ARGP_IN_ORDER)
    *cvt.short_end++ = '-';
  else if (flags & ARGP_NO_ARGS)
    *cvt.short_end++ = '+';
  *cvt.short_end = '\0';

  cvt.long_end->name = NULL;

  parser->argp = argp;

  if (argp)
    parser->egroup = convert_options (argp, 0, 0, parser->groups, &cvt);
  else
    parser->egroup = parser->groups; /* No parsers at all! */
}

/* Lengths of various parser fields which we will mallocted.  */
struct parser_sizes
{
  size_t short_len;		/* Getopt short options string.  */
  size_t long_len;		/* Getopt long options vector.  */
  size_t num_groups;		/* Group structures we mallocte.  */
  size_t num_child_inputs;	/* Child input slots.  */
};

/* For ARGP, increments the NUM_GROUPS field in SZS by the total number of
 argp structures descended from it, and the SHORT_LEN & LONG_LEN fields by
 the maximum lengths of the resulting merged getopt short options string and
 long-options array, respectively.  */
static void
calc_sizes (const struct argp *argp,  struct parser_sizes *szs)
{
  const struct argp_child *child = argp->children;
  const struct argp_option *opt = argp->options;

  if (opt || argp->parser)
    {
      szs->num_groups++;
      if (opt)
	{
	  int num_opts = 0;
	  while (!_option_is_end (opt++))
	    num_opts++;
	  szs->short_len += num_opts * 3; /* opt + up to 2 `:'s */
	  szs->long_len += num_opts;
	}
    }

  if (child)
    while (child->argp)
      {
	calc_sizes ((child++)->argp, szs);
	szs->num_child_inputs++;
      }
}

/* Initializes PARSER to parse ARGP in a manner described by FLAGS.  */
static error_t
parser_init (struct parser *parser, const struct argp *argp,
	     int argc, char **argv, int flags, void *input)
{
  error_t err = 0;
  struct group *group;
  struct parser_sizes szs;
  struct _getopt_data opt_data = _GETOPT_DATA_INITIALIZER;

  szs.short_len = (flags & ARGP_NO_ARGS) ? 0 : 1;
  szs.long_len = 0;
  szs.num_groups = 0;
  szs.num_child_inputs = 0;

  if (argp)
    calc_sizes (argp, &szs);

  /* Lengths of the various bits of storage used by PARSER.  */
#define GLEN (szs.num_groups + 1) * sizeof (struct group)
#define CLEN (szs.num_child_inputs * sizeof (void *))
#define LLEN ((szs.long_len + 1) * sizeof (struct option))
#define SLEN (szs.short_len + 1)

  parser->storage = malloc (GLEN + CLEN + LLEN + SLEN);
  if (! parser->storage)
    return ENOMEM;

  parser->groups       = parser->storage;
  parser->child_inputs = (char*)parser->storage + GLEN;
  parser->long_opts    = (char*)parser->storage + GLEN + CLEN;
  parser->short_opts   = (char*)parser->storage + GLEN + CLEN + LLEN;
  parser->opt_data     = opt_data;

  memset (parser->child_inputs, 0, szs.num_child_inputs * sizeof (void *));
  parser_convert (parser, argp, flags);

  memset (&parser->state, 0, sizeof (struct argp_state));
  parser->state.root_argp = parser->argp;
  parser->state.argc = argc;
  parser->state.argv = argv;
  parser->state.flags = flags;
  parser->state.err_stream = stderr;
  parser->state.out_stream = stdout;
  parser->state.next = 0;	/* Tell getopt to initialize.  */
  parser->state.pstate = parser;

  parser->try_getopt = 1;

  /* Call each parser for the first time, giving it a chance to propagate
     values to child parsers.  */
  if (parser->groups < parser->egroup)
    parser->groups->input = input;
  for (group = parser->groups;
       group < parser->egroup && (!err || err == EBADKEY);
       group++)
    {
      if (group->parent)
	/* If a child parser, get the initial input value from the parent. */
	group->input = group->parent->child_inputs[group->parent_index];

      if (!group->parser
	  && group->argp->children && group->argp->children->argp)
	/* For the special case where no parsing function is supplied for an
	   argp, propagate its input to its first child, if any (this just
	   makes very simple wrapper argps more convenient).  */
	group->child_inputs[0] = group->input;

      err = group_parse (group, &parser->state, ARGP_KEY_INIT, 0);
    }
  if (err == EBADKEY)
    err = 0;			/* Some parser didn't understand.  */

  if (err)
    return err;

  if (parser->state.flags & ARGP_NO_ERRS)
    {
      parser->opt_data.opterr = 0;
      if (parser->state.flags & ARGP_PARSE_ARGV0)
	/* getopt always skips ARGV[0], so we have to fake it out.  As long
	   as OPTERR is 0, then it shouldn't actually try to access it.  */
	parser->state.argv--, parser->state.argc++;
    }
  else
    parser->opt_data.opterr = 1;	/* Print error messages.  */

  if (parser->state.argv == argv && argv[0])
    /* There's an argv[0]; use it for messages.  */
    {
      char *short_name = strrchr (argv[0], '/');
      parser->state.name = short_name ? short_name + 1 : argv[0];
    }
  else
    parser->state.name = "";

  return 0;
}

/* Free any storage consumed by PARSER (but not PARSER itself).  */
static error_t
parser_finalize (struct parser *parser,
		 error_t err, int arg_ebadkey, int *end_index)
{
  struct group *group;

  if (err == EBADKEY && arg_ebadkey)
    /* Suppress errors generated by unparsed arguments.  */
    err = 0;

  if (! err)
    {
      if (parser->state.next == parser->state.argc)
	/* We successfully parsed all arguments!  Call all the parsers again,
	   just a few more times... */
	{
	  for (group = parser->groups;
	       group < parser->egroup && (!err || err==EBADKEY);
	       group++)
	    if (group->args_processed == 0)
	      err = group_parse (group, &parser->state, ARGP_KEY_NO_ARGS, 0);
	  for (group = parser->egroup - 1;
	       group >= parser->groups && (!err || err==EBADKEY);
	       group--)
	    err = group_parse (group, &parser->state, ARGP_KEY_END, 0);

	  if (err == EBADKEY)
	    err = 0;		/* Some parser didn't understand.  */

	  /* Tell the user that all arguments are parsed.  */
	  if (end_index)
	    *end_index = parser->state.next;
	}
      else if (end_index)
	/* Return any remaining arguments to the user.  */
	*end_index = parser->state.next;
      else
	/* No way to return the remaining arguments, they must be bogus. */
	{
	  if (!(parser->state.flags & ARGP_NO_ERRS)
	      && parser->state.err_stream)
	    fprintf (parser->state.err_stream,
			       "%s: Too many arguments\n",
		     parser->state.name);
	  err = EBADKEY;
	}
    }

  /* Okay, we're all done, with either an error or success; call the parsers
     to indicate which one.  */

  if (err)
    {
      /* Maybe print an error message.  */
      if (err == EBADKEY)
	/* An appropriate message describing what the error was should have
	   been printed earlier.  */
	argp_state_help (&parser->state, parser->state.err_stream,
			   ARGP_HELP_STD_ERR);

      /* Since we didn't exit, give each parser an error indication.  */
      for (group = parser->groups; group < parser->egroup; group++)
	group_parse (group, &parser->state, ARGP_KEY_ERROR, 0);
    }
  else
    /* Notify parsers of success, and propagate back values from parsers.  */
    {
      /* We pass over the groups in reverse order so that child groups are
	 given a chance to do there processing before passing back a value to
	 the parent.  */
      for (group = parser->egroup - 1
	   ; group >= parser->groups && (!err || err == EBADKEY)
	   ; group--)
	err = group_parse (group, &parser->state, ARGP_KEY_SUCCESS, 0);
      if (err == EBADKEY)
	err = 0;		/* Some parser didn't understand.  */
    }

  /* Call parsers once more, to do any final cleanup.  Errors are ignored.  */
  for (group = parser->egroup - 1; group >= parser->groups; group--)
    group_parse (group, &parser->state, ARGP_KEY_FINI, 0);

  if (err == EBADKEY)
    err = EINVAL;

  free (parser->storage);

  return err;
}

/* Call the user parsers to parse the non-option argument VAL, at the current
   position, returning any error.  The state NEXT pointer is assumed to have
   been adjusted (by getopt) to point after this argument; this function will
   adjust it correctly to reflect however many args actually end up being
   consumed.  */
static error_t
parser_parse_arg (struct parser *parser, char *val)
{
  /* Save the starting value of NEXT, first adjusting it so that the arg
     we're parsing is again the front of the arg vector.  */
  int index = --parser->state.next;
  error_t err = EBADKEY;
  struct group *group;
  int key = 0;			/* Which of ARGP_KEY_ARG[S] we used.  */

  /* Try to parse the argument in each parser.  */
  for (group = parser->groups
       ; group < parser->egroup && err == EBADKEY
       ; group++)
    {
      parser->state.next++;	/* For ARGP_KEY_ARG, consume the arg.  */
      key = ARGP_KEY_ARG;
      err = group_parse (group, &parser->state, key, val);

      if (err == EBADKEY)
	/* This parser doesn't like ARGP_KEY_ARG; try ARGP_KEY_ARGS instead. */
	{
	  parser->state.next--;	/* For ARGP_KEY_ARGS, put back the arg.  */
	  key = ARGP_KEY_ARGS;
	  err = group_parse (group, &parser->state, key, 0);
	}
    }

  if (! err)
    {
      if (key == ARGP_KEY_ARGS)
	/* The default for ARGP_KEY_ARGS is to assume that if NEXT isn't
	   changed by the user, *all* arguments should be considered
	   consumed.  */
	parser->state.next = parser->state.argc;

      if (parser->state.next > index)
	/* Remember that we successfully processed a non-option
	   argument -- but only if the user hasn't gotten tricky and set
	   the clock back.  */
	(--group)->args_processed += (parser->state.next - index);
      else
	/* The user wants to reparse some args, give getopt another try.  */
	parser->try_getopt = 1;
    }

  return err;
}

/* Call the user parsers to parse the option OPT, with argument VAL, at the
   current position, returning any error.  */
static error_t
parser_parse_opt (struct parser *parser, int opt, char *val)
{
  /* The group key encoded in the high bits; 0 for short opts or
     group_number + 1 for long opts.  */
  int group_key = opt >> USER_BITS;
  error_t err = EBADKEY;

  if (group_key == 0)
    /* A short option.  By comparing OPT's position in SHORT_OPTS to the
       various starting positions in each group's SHORT_END field, we can
       determine which group OPT came from.  */
    {
      struct group *group;
      char *short_index = strchr (parser->short_opts, opt);

      if (short_index)
	for (group = parser->groups; group < parser->egroup; group++)
	  if (group->short_end > short_index)
	    {
	      err = group_parse (group, &parser->state, opt,
				 parser->opt_data.optarg);
	      break;
	    }
    }
  else
    /* A long option.  We use shifts instead of masking for extracting
       the user value in order to preserve the sign.  */
    err =
      group_parse (&parser->groups[group_key - 1], &parser->state,
		   (opt << GROUP_BITS) >> GROUP_BITS,
		   parser->opt_data.optarg);

  if (err == EBADKEY)
    /* At least currently, an option not recognized is an error in the
       parser, because we pre-compute which parser is supposed to deal
       with each option.  */
    {
      static const char bad_key_err[] =
	"(PROGRAM ERROR) Option should have been recognized!?";
      if (group_key == 0)
	argp_error (&parser->state, "-%c: %s", opt,
		      bad_key_err);
      else
	{
	  struct option *long_opt = parser->long_opts;
	  while (long_opt->val != opt && long_opt->name)
	    long_opt++;
	  argp_error (&parser->state, "--%s: %s",
			long_opt->name ? long_opt->name : "???",
			bad_key_err);
	}
    }

  return err;
}

/* Parse the next argument in PARSER (as indicated by PARSER->state.next).
   Any error from the parsers is returned, and *ARGP_EBADKEY indicates
   whether a value of EBADKEY is due to an unrecognized argument (which is
   generally not fatal).  */
static error_t
parser_parse_next (struct parser *parser, int *arg_ebadkey)
{
  int opt;
  error_t err = 0;

  if (parser->state.quoted && parser->state.next < parser->state.quoted)
    /* The next argument pointer has been moved to before the quoted
       region, so pretend we never saw the quoting `--', and give getopt
       another chance.  If the user hasn't removed it, getopt will just
       process it again.  */
    parser->state.quoted = 0;

  if (parser->try_getopt && !parser->state.quoted)
    /* Give getopt a chance to parse this.  */
    {
      /* Put it back in OPTIND for getopt.  */
      parser->opt_data.optind = parser->state.next;
      /* Distinguish KEY_ERR from a real option.  */
      parser->opt_data.optopt = KEY_END;
      if (parser->state.flags & ARGP_LONG_ONLY)
	opt = _getopt_long_only_r (parser->state.argc, parser->state.argv,
				   parser->short_opts, parser->long_opts, 0,
				   &parser->opt_data);
      else
	opt = _getopt_long_r (parser->state.argc, parser->state.argv,
			      parser->short_opts, parser->long_opts, 0,
			      &parser->opt_data);
      /* And see what getopt did.  */
      parser->state.next = parser->opt_data.optind;

      if (opt == KEY_END)
	/* Getopt says there are no more options, so stop using
	   getopt; we'll continue if necessary on our own.  */
	{
	  parser->try_getopt = 0;
	  if (parser->state.next > 1
	      && strcmp (parser->state.argv[parser->state.next - 1], QUOTE)
	           == 0)
	    /* Not only is this the end of the options, but it's a
	       `quoted' region, which may have args that *look* like
	       options, so we definitely shouldn't try to use getopt past
	       here, whatever happens.  */
	    parser->state.quoted = parser->state.next;
	}
      else if (opt == KEY_ERR && parser->opt_data.optopt != KEY_END)
	/* KEY_ERR can have the same value as a valid user short
	   option, but in the case of a real error, getopt sets OPTOPT
	   to the offending character, which can never be KEY_END.  */
	{
	  *arg_ebadkey = 0;
	  return EBADKEY;
	}
    }
  else
    opt = KEY_END;

  if (opt == KEY_END)
    {
      /* We're past what getopt considers the options.  */
      if (parser->state.next >= parser->state.argc
	  || (parser->state.flags & ARGP_NO_ARGS))
	/* Indicate that we're done.  */
	{
	  *arg_ebadkey = 1;
	  return EBADKEY;
	}
      else
	/* A non-option arg; simulate what getopt might have done.  */
	{
	  opt = KEY_ARG;
	  parser->opt_data.optarg = parser->state.argv[parser->state.next++];
	}
    }

  if (opt == KEY_ARG)
    /* A non-option argument; try each parser in turn.  */
    err = parser_parse_arg (parser, parser->opt_data.optarg);
  else
    err = parser_parse_opt (parser, opt, parser->opt_data.optarg);

  if (err == EBADKEY)
    *arg_ebadkey = (opt == KEY_END || opt == KEY_ARG);

  return err;
}

/* Parse the options strings in ARGC & ARGV according to the argp in ARGP.
   FLAGS is one of the ARGP_ flags above.  If END_INDEX is non-NULL, the
   index in ARGV of the first unparsed option is returned in it.  If an
   unknown option is present, EINVAL is returned; if some parser routine
   returned a non-zero value, it is returned; otherwise 0 is returned.  */
error_t
argp_parse (const struct argp *argp, int argc, char **argv, unsigned flags,
	      int *end_index, void *input)
{
  error_t err;
  struct parser parser;

  /* If true, then err == EBADKEY is a result of a non-option argument failing
     to be parsed (which in some cases isn't actually an error).  */
  int arg_ebadkey = 0;

  if (! (flags & ARGP_NO_HELP))
    /* Add our own options.  */
    {
      struct argp_child *child = malloc (4 * sizeof (struct argp_child));
      struct argp *top_argp = malloc (sizeof (struct argp));

      /* TOP_ARGP has no options, it just serves to group the user & default
	 argps.  */
      memset (top_argp, 0, sizeof (*top_argp));
      top_argp->children = child;

      memset (child, 0, 4 * sizeof (struct argp_child));

      if (argp)
	(child++)->argp = argp;
      (child++)->argp = &argp_default_argp;
      if (argp_program_version || argp_program_version_hook)
	(child++)->argp = &argp_version_argp;
      child->argp = 0;

      argp = top_argp;
    }

  /* Construct a parser for these arguments.  */
  err = parser_init (&parser, argp, argc, argv, flags, input);

  if (! err)
    /* Parse! */
    {
      while (! err)
	err = parser_parse_next (&parser, &arg_ebadkey);
      err = parser_finalize (&parser, err, arg_ebadkey, end_index);
    }

  return err;
}

/* Return the input field for ARGP in the parser corresponding to STATE; used
   by the help routines.  */
void *
argp_input (const struct argp *argp, const struct argp_state *state)
{
  if (state)
    {
      struct group *group;
      struct parser *parser = state->pstate;

      for (group = parser->groups; group < parser->egroup; group++)
	if (group->argp == argp)
	  return group->input;
    }

  return 0;
}

int _option_is_short (const struct argp_option *opt)
{
  if (opt->flags & OPTION_DOC)
    return 0;
  else
    {
      int __key = opt->key;
      return __key > 0 && __key <= UCHAR_MAX && isprint (__key);
    }
}

int _option_is_end(const struct argp_option *opt)
{
  return !opt->key && !opt->name && !opt->doc && !opt->group;
}

size_t
argp_fmtstream_write (argp_fmtstream_t __fs,
			const char *__str, size_t __len)
{
  if (__fs->p + __len <= __fs->end || argp_fmtstream_ensure (__fs, __len))
    {
      memcpy (__fs->p, __str, __len);
      __fs->p += __len;
      return __len;
    }
  else
    return 0;
}

int
argp_fmtstream_puts (argp_fmtstream_t __fs, const char *__str)
{
  size_t __len = strlen (__str);
  if (__len)
    {
      size_t __wrote = argp_fmtstream_write (__fs, __str, __len);
      return __wrote == __len ? 0 : -1;
    }
  else
    return 0;
}

int argp_fmtstream_putc (argp_fmtstream_t __fs, int __ch)
{
  if (__fs->p < __fs->end || argp_fmtstream_ensure (__fs, 1))
    return *__fs->p++ = __ch;
  else
    return EOF;
}

/* Set __FS's left margin to __LMARGIN and return the old value.  */
size_t argp_fmtstream_set_lmargin (argp_fmtstream_t __fs, size_t __lmargin)
{
  size_t __old;
  if ((size_t) (__fs->p - __fs->buf) > __fs->point_offs)
    argp_fmtstream_update (__fs);
  __old = __fs->lmargin;
  __fs->lmargin = __lmargin;
  return __old;
}

/* Set __FS's right margin to __RMARGIN and return the old value.  */
size_t argp_fmtstream_set_rmargin (argp_fmtstream_t __fs, size_t __rmargin)
{
  size_t __old;
  if ((size_t) (__fs->p - __fs->buf) > __fs->point_offs)
    argp_fmtstream_update (__fs);
  __old = __fs->rmargin;
  __fs->rmargin = __rmargin;
  return __old;
}

/* Set FS's wrap margin to __WMARGIN and return the old value.  */
size_t argp_fmtstream_set_wmargin (argp_fmtstream_t __fs, size_t __wmargin)
{
  size_t __old;
  if ((size_t) (__fs->p - __fs->buf) > __fs->point_offs)
    argp_fmtstream_update (__fs);
  __old = __fs->wmargin;
  __fs->wmargin = __wmargin;
  return __old;
}

/* Return the column number of the current output point in __FS.  */
size_t argp_fmtstream_point (argp_fmtstream_t __fs)
{
  if ((size_t) (__fs->p - __fs->buf) > __fs->point_offs)
    argp_fmtstream_update (__fs);
  return __fs->point_col >= 0 ? __fs->point_col : 0;
}

#ifndef isblank
#define isblank(ch) ((ch)==' ' || (ch)=='\t')
#endif

#define INIT_BUF_SIZE 200
#define PRINTF_SIZE_GUESS 150

/* Return an argp_fmtstream that outputs to STREAM, and which prefixes lines
   written on it with LMARGIN spaces and limits them to RMARGIN columns
   total.  If WMARGIN >= 0, words that extend past RMARGIN are wrapped by
   replacing the whitespace before them with a newline and WMARGIN spaces.
   Otherwise, chars beyond RMARGIN are simply dropped until a newline.
   Returns NULL if there was an error.  */
argp_fmtstream_t
argp_make_fmtstream (FILE *stream,
		       size_t lmargin, size_t rmargin, ssize_t wmargin)
{
  argp_fmtstream_t fs;

  fs = (struct argp_fmtstream *) malloc (sizeof (struct argp_fmtstream));
  if (fs != NULL)
    {
      fs->stream = stream;

      fs->lmargin = lmargin;
      fs->rmargin = rmargin;
      fs->wmargin = wmargin;
      fs->point_col = 0;
      fs->point_offs = 0;

      fs->buf = (char *) malloc (INIT_BUF_SIZE);
      if (! fs->buf)
	{
	  free (fs);
	  fs = 0;
	}
      else
	{
	  fs->p = fs->buf;
	  fs->end = fs->buf + INIT_BUF_SIZE;
	}
    }

  return fs;
}

/* Flush FS to its stream, and free it (but don't close the stream).  */
void
argp_fmtstream_free (argp_fmtstream_t fs)
{
  argp_fmtstream_update (fs);
  if (fs->p > fs->buf)
    {
      fwrite (fs->buf, 1, fs->p - fs->buf, fs->stream);
    }
  free (fs->buf);
  free (fs);
}

/* Process FS's buffer so that line wrapping is done from POINT_OFFS to the
   end of its buffer.  This code is mostly from glibc stdio/linewrap.c.  */
void
argp_fmtstream_update (argp_fmtstream_t fs)
{
  char *buf, *nl;
  size_t len;

  /* Scan the buffer for newlines.  */
  buf = fs->buf + fs->point_offs;
  while (buf < fs->p)
    {
      size_t r;

      if (fs->point_col == 0 && fs->lmargin != 0)
	{
	  /* We are starting a new line.  Print spaces to the left margin.  */
	  const size_t pad = fs->lmargin;
	  if (fs->p + pad < fs->end)
	    {
	      /* We can fit in them in the buffer by moving the
		 buffer text up and filling in the beginning.  */
	      memmove (buf + pad, buf, fs->p - buf);
	      fs->p += pad; /* Compensate for bigger buffer. */
	      memset (buf, ' ', pad); /* Fill in the spaces.  */
	      buf += pad; /* Don't bother searching them.  */
	    }
	  else
	    {
	      /* No buffer space for spaces.  Must flush.  */
	      size_t i;
	      for (i = 0; i < pad; i++)
		{
		    putc (' ', fs->stream);
		}
	    }
	  fs->point_col = pad;
	}

      len = fs->p - buf;
      nl = memchr (buf, '\n', len);

      if (fs->point_col < 0)
	fs->point_col = 0;

      if (!nl)
	{
	  /* The buffer ends in a partial line.  */

	  if (fs->point_col + len < fs->rmargin)
	    {
	      /* The remaining buffer text is a partial line and fits
		 within the maximum line width.  Advance point for the
		 characters to be written and stop scanning.  */
	      fs->point_col += len;
	      break;
	    }
	  else
	    /* Set the end-of-line pointer for the code below to
	       the end of the buffer.  */
	    nl = fs->p;
	}
      else if (fs->point_col + (nl - buf) < (ssize_t) fs->rmargin)
	{
	  /* The buffer contains a full line that fits within the maximum
	     line width.  Reset point and scan the next line.  */
	  fs->point_col = 0;
	  buf = nl + 1;
	  continue;
	}

      /* This line is too long.  */
      r = fs->rmargin - 1;

      if (fs->wmargin < 0)
	{
	  /* Truncate the line by overwriting the excess with the
	     newline and anything after it in the buffer.  */
	  if (nl < fs->p)
	    {
	      memmove (buf + (r - fs->point_col), nl, fs->p - nl);
	      fs->p -= buf + (r - fs->point_col) - nl;
	      /* Reset point for the next line and start scanning it.  */
	      fs->point_col = 0;
	      buf += r + 1; /* Skip full line plus \n. */
	    }
	  else
	    {
	      /* The buffer ends with a partial line that is beyond the
		 maximum line width.  Advance point for the characters
		 written, and discard those past the max from the buffer.  */
	      fs->point_col += len;
	      fs->p -= fs->point_col - r;
	      break;
	    }
	}
      else
	{
	  /* Do word wrap.  Go to the column just past the maximum line
	     width and scan back for the beginning of the word there.
	     Then insert a line break.  */

	  char *p, *nextline;
	  int i;

	  p = buf + (r + 1 - fs->point_col);
	  while (p >= buf && !isblank (*p))
	    --p;
	  nextline = p + 1;	/* This will begin the next line.  */

	  if (nextline > buf)
	    {
	      /* Swallow separating blanks.  */
	      if (p >= buf)
		do
		  --p;
		while (p >= buf && isblank (*p));
	      nl = p + 1;	/* The newline will replace the first blank. */
	    }
	  else
	    {
	      /* A single word that is greater than the maximum line width.
		 Oh well.  Put it on an overlong line by itself.  */
	      p = buf + (r + 1 - fs->point_col);
	      /* Find the end of the long word.  */
	      do
		++p;
	      while (p < nl && !isblank (*p));
	      if (p == nl)
		{
		  /* It already ends a line.  No fussing required.  */
		  fs->point_col = 0;
		  buf = nl + 1;
		  continue;
		}
	      /* We will move the newline to replace the first blank.  */
	      nl = p;
	      /* Swallow separating blanks.  */
	      do
		++p;
	      while (isblank (*p));
	      /* The next line will start here.  */
	      nextline = p;
	    }

	  /* Note: There are a bunch of tests below for
	     NEXTLINE == BUF + LEN + 1; this case is where NL happens to fall
	     at the end of the buffer, and NEXTLINE is in fact empty (and so
	     we need not be careful to maintain its contents).  */

	  if ((nextline == buf + len + 1
	       ? fs->end - nl < fs->wmargin + 1
	       : nextline - (nl + 1) < fs->wmargin)
	      && fs->p > nextline)
	    {
	      /* The margin needs more blanks than we removed.  */
	      if (fs->end - fs->p > fs->wmargin + 1)
		/* Make some space for them.  */
		{
		  size_t mv = fs->p - nextline;
		  memmove (nl + 1 + fs->wmargin, nextline, mv);
		  nextline = nl + 1 + fs->wmargin;
		  len = nextline + mv - buf;
		  *nl++ = '\n';
		}
	      else
		/* Output the first line so we can use the space.  */
		{
		  if (nl > fs->buf)
		    fwrite (fs->buf, 1, nl - fs->buf, fs->stream);
		  putc ('\n', fs->stream);

		  len += buf - fs->buf;
		  nl = buf = fs->buf;
		}
	    }
	  else
	    /* We can fit the newline and blanks in before
	       the next word.  */
	    *nl++ = '\n';

	  if (nextline - nl >= fs->wmargin
	      || (nextline == buf + len + 1 && fs->end - nextline >= fs->wmargin))
	    /* Add blanks up to the wrap margin column.  */
	    for (i = 0; i < fs->wmargin; ++i)
	      *nl++ = ' ';
	  else
	    for (i = 0; i < fs->wmargin; ++i)
		putc (' ', fs->stream);

	  /* Copy the tail of the original buffer into the current buffer
	     position.  */
	  if (nl < nextline)
	    memmove (nl, nextline, buf + len - nextline);
	  len -= nextline - buf;

	  /* Continue the scan on the remaining lines in the buffer.  */
	  buf = nl;

	  /* Restore bufp to include all the remaining text.  */
	  fs->p = nl + len;

	  /* Reset the counter of what has been output this line.  If wmargin
	     is 0, we want to avoid the lmargin getting added, so we set
	     point_col to a magic value of -1 in that case.  */
	  fs->point_col = fs->wmargin ? fs->wmargin : -1;
	}
    }

  /* Remember that we've scanned as far as the end of the buffer.  */
  fs->point_offs = fs->p - fs->buf;
}

/* Ensure that FS has space for AMOUNT more bytes in its buffer, either by
   growing the buffer, or by flushing it.  True is returned iff we succeed. */
int
argp_fmtstream_ensure (struct argp_fmtstream *fs, size_t amount)
{
  if ((size_t) (fs->end - fs->p) < amount)
    {
      ssize_t wrote;

      /* Flush FS's buffer.  */
      argp_fmtstream_update (fs);

      wrote = fwrite (fs->buf, 1, fs->p - fs->buf, fs->stream);
      if (wrote == fs->p - fs->buf)
	{
	  fs->p = fs->buf;
	  fs->point_offs = 0;
	}
      else
	{
	  fs->p -= wrote;
	  fs->point_offs -= wrote;
	  memmove (fs->buf, fs->buf + wrote, fs->p - fs->buf);
	  return 0;
	}

      if ((size_t) (fs->end - fs->buf) < amount)
	/* Gotta grow the buffer.  */
	{
	  size_t old_size = fs->end - fs->buf;
	  size_t new_size = old_size + amount;
	  char *new_buf;

	  if (new_size < old_size || ! (new_buf = realloc (fs->buf, new_size)))
	    {
//	      __set_errno (ENOMEM);
	      return 0;
	    }

	  fs->buf = new_buf;
	  fs->end = new_buf + new_size;
	  fs->p = fs->buf;
	}
    }

  return 1;
}

ssize_t
argp_fmtstream_printf (struct argp_fmtstream *fs, const char *fmt, ...)
{
  int out;
  size_t avail;
  size_t size_guess = PRINTF_SIZE_GUESS; /* How much space to reserve. */

  do
    {
      va_list args;

      if (! argp_fmtstream_ensure (fs, size_guess))
	return -1;

      va_start (args, fmt);
      avail = fs->end - fs->p;
      out = _vsnprintf (fs->p, avail, fmt, args);
      va_end (args);
      if ((size_t) out >= avail)
	size_guess = out + 1;
    }
  while ((size_t) out >= avail);

  fs->p += out;

  return out;
}

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

/* User-selectable (using an environment variable) formatting parameters.

   These may be specified in an environment variable called `ARGP_HELP_FMT',
   with a contents like:  VAR1=VAL1,VAR2=VAL2,BOOLVAR2,no-BOOLVAR2
   Where VALn must be a positive integer.  The list of variables is in the
   UPARAM_NAMES vector, below.  */

/* Default parameters.  */
#define DUP_ARGS      0		/* True if option argument can be duplicated. */
#define DUP_ARGS_NOTE 1		/* True to print a note about duplicate args. */
#define SHORT_OPT_COL 2		/* column in which short options start */
#define LONG_OPT_COL  6		/* column in which long options start */
#define DOC_OPT_COL   2		/* column in which doc options start */
#define OPT_DOC_COL  29		/* column in which option text starts */
#define HEADER_COL    1		/* column in which group headers are printed */
#define USAGE_INDENT 12		/* indentation of wrapped usage lines */
#define RMARGIN      79		/* right margin used for wrapping */

/* User-selectable (using an environment variable) formatting parameters.
   They must all be of type `int' for the parsing code to work.  */
struct uparams
{
  /* If true, arguments for an option are shown with both short and long
     options, even when a given option has both, e.g. `-x ARG, --longx=ARG'.
     If false, then if an option has both, the argument is only shown with
     the long one, e.g., `-x, --longx=ARG', and a message indicating that
     this really means both is printed below the options.  */
  int dup_args;

  /* This is true if when DUP_ARGS is false, and some duplicate arguments have
     been suppressed, an explanatory message should be printed.  */
  int dup_args_note;

  /* Various output columns.  */
  int short_opt_col;
  int long_opt_col;
  int doc_opt_col;
  int opt_doc_col;
  int header_col;
  int usage_indent;
  int rmargin;
};

/* This is a global variable, as user options are only ever read once.  */
static struct uparams uparams = {
  DUP_ARGS, DUP_ARGS_NOTE,
  SHORT_OPT_COL, LONG_OPT_COL, DOC_OPT_COL, OPT_DOC_COL, HEADER_COL,
  USAGE_INDENT, RMARGIN
};

/* A particular uparam, and what the user name is.  */
struct uparam_name
{
  const char name[14];		/* User name.  */
  int is_bool;			/* Whether it's `boolean'.  */
  uint8_t uparams_offs;		/* Location of the (int) field in UPARAMS.  */
};

/* The name-field mappings we know about.  */
static const struct uparam_name uparam_names[] =
{
  { "dup-args",       1, offsetof (struct uparams, dup_args) },
  { "dup-args-note",  1, offsetof (struct uparams, dup_args_note) },
  { "short-opt-col",  0, offsetof (struct uparams, short_opt_col) },
  { "long-opt-col",   0, offsetof (struct uparams, long_opt_col) },
  { "doc-opt-col",    0, offsetof (struct uparams, doc_opt_col) },
  { "opt-doc-col",    0, offsetof (struct uparams, opt_doc_col) },
  { "header-col",     0, offsetof (struct uparams, header_col) },
  { "usage-indent",   0, offsetof (struct uparams, usage_indent) },
  { "rmargin",        0, offsetof (struct uparams, rmargin) }
};
#define nuparam_names (sizeof (uparam_names) / sizeof (uparam_names[0]))

/* Read user options from the environment, and fill in UPARAMS appropiately.  */
static void
fill_in_uparams (const struct argp_state *state)
{  
  const char *var = getenv ("ARGP_HELP_FMT");

  size_t u;

#define SKIPWS(p) do { while (isspace (*p)) p++; } while (0);

  if (var)
    /* Parse var. */
    while (*var)
      {
	SKIPWS (var);

	if (isalpha (*var))
	  {
	    size_t var_len;
	    const struct uparam_name *un;
	    int unspec = 0, val = 0;
	    const char *arg = var;

	    while (isalnum (*arg) || *arg == '-' || *arg == '_')
	      arg++;
	    var_len = arg - var;

	    SKIPWS (arg);

	    if (*arg == '\0' || *arg == ',')
	      unspec = 1;
	    else if (*arg == '=')
	      {
		arg++;
		SKIPWS (arg);
	      }

	    if (unspec)
	      {
		if (var[0] == 'n' && var[1] == 'o' && var[2] == '-')
		  {
		    val = 0;
		    var += 3;
		    var_len -= 3;
		  }
		else
		  val = 1;
	      }
	    else if (isdigit (*arg))
	      {
		val = atoi (arg);
		while (isdigit (*arg))
		  arg++;
		SKIPWS (arg);
	      }

	    un = uparam_names;
	    
	    for (u = 0; u < nuparam_names; ++un, ++u)
	      if (strlen (un->name) == var_len
		  && strncmp (var, un->name, var_len) == 0)
		{
		  if (unspec && !un->is_bool){
		    argp_failure(state, 0, 0,"%d.%s: ARGP_HELP_FMT parameter requires a value",
			      (int) var_len, var);
		  }
		  else
		    *(int *)((char *)&uparams + un->uparams_offs) = val;
		  break;
		}
	    if (u == nuparam_names)
	      argp_failure (state, 0, 0,
			      "%d.%s: Unknown ARGP_HELP_FMT parameter",
			      (int) var_len, var);

	    var = arg;
	    if (*var == ',')
	      var++;
	  }
	else if (*var)
	  {
	    argp_failure (state, 0, 0,
			    "Garbage in ARGP_HELP_FMT: %s", var);
	    break;
	  }
      }
}

/* Returns true if OPT hasn't been marked invisible.  Visibility only affects
   whether OPT is displayed or used in sorting, not option shadowing.  */
#define ovisible(opt) (! ((opt)->flags & OPTION_HIDDEN))

/* Returns true if OPT is an alias for an earlier option.  */
#define oalias(opt) ((opt)->flags & OPTION_ALIAS)

/* Returns true if OPT is an documentation-only entry.  */
#define odoc(opt) ((opt)->flags & OPTION_DOC)

/* Returns true if OPT is the end-of-list marker for a list of options.  */
#define oend(opt) _option_is_end (opt)

/* Returns true if OPT has a short option.  */
#define oshort(opt) _option_is_short (opt)

/*
   The help format for a particular option is like:

     -xARG, -yARG, --long1=ARG, --long2=ARG        Documentation...

   Where ARG will be omitted if there's no argument, for this option, or
   will be surrounded by "[" and "]" appropiately if the argument is
   optional.  The documentation string is word-wrapped appropiately, and if
   the list of options is long enough, it will be started on a separate line.
   If there are no short options for a given option, the first long option is
   indented slighly in a way that's supposed to make most long options appear
   to be in a separate column.

   For example, the following output (from ps):

     -p PID, --pid=PID          List the process PID
	 --pgrp=PGRP            List processes in the process group PGRP
     -P, -x, --no-parent        Include processes without parents
     -Q, --all-fields           Don't elide unusable fields (normally if there's
				some reason ps can't print a field for any
				process, it's removed from the output entirely)
     -r, --reverse, --gratuitously-long-reverse-option
				Reverse the order of any sort
	 --session[=SID]        Add the processes from the session SID (which
				defaults to the sid of the current process)

    Here are some more options:
     -f ZOT, --foonly=ZOT       Glork a foonly
     -z, --zaza                 Snit a zar

     -?, --help                 Give this help list
	 --usage                Give a short usage message
     -V, --version              Print program version

   The struct argp_option array for the above could look like:

   {
     {"pid",       'p',      "PID",  0, "List the process PID"},
     {"pgrp",      OPT_PGRP, "PGRP", 0, "List processes in the process group PGRP"},
     {"no-parent", 'P',	      0,     0, "Include processes without parents"},
     {0,           'x',       0,     OPTION_ALIAS},
     {"all-fields",'Q',       0,     0, "Don't elide unusable fields (normally"
                                        " if there's some reason ps can't"
					" print a field for any process, it's"
                                        " removed from the output entirely)" },
     {"reverse",   'r',       0,     0, "Reverse the order of any sort"},
     {"gratuitously-long-reverse-option", 0, 0, OPTION_ALIAS},
     {"session",   OPT_SESS,  "SID", OPTION_ARG_OPTIONAL,
                                        "Add the processes from the session"
					" SID (which defaults to the sid of"
					" the current process)" },

     {0,0,0,0, "Here are some more options:"},
     {"foonly", 'f', "ZOT", 0, "Glork a foonly"},
     {"zaza", 'z', 0, 0, "Snit a zar"},

     {0}
   }

   Note that the last three options are automatically supplied by argp_parse,
   unless you tell it not to with ARGP_NO_HELP.

*/

/* Returns true if CH occurs between BEG and END.  */
static int
find_char (char ch, char *beg, char *end)
{
  while (beg < end)
    if (*beg == ch)
      return 1;
    else
      beg++;
  return 0;
}

struct hol_cluster;		/* fwd decl */

struct hol_entry
{
  /* First option.  */
  const struct argp_option *opt;
  /* Number of options (including aliases).  */
  unsigned num;

  /* A pointers into the HOL's short_options field, to the first short option
     letter for this entry.  The order of the characters following this point
     corresponds to the order of options pointed to by OPT, and there are at
     most NUM.  A short option recorded in a option following OPT is only
     valid if it occurs in the right place in SHORT_OPTIONS (otherwise it's
     probably been shadowed by some other entry).  */
  char *short_options;

  /* Entries are sorted by their group first, in the order:
       1, 2, ..., n, 0, -m, ..., -2, -1
     and then alphabetically within each group.  The default is 0.  */
  int group;

  /* The cluster of options this entry belongs to, or 0 if none.  */
  struct hol_cluster *cluster;

  /* The argp from which this option came.  */
  const struct argp *argp;
};

/* A cluster of entries to reflect the argp tree structure.  */
struct hol_cluster
{
  /* A descriptive header printed before options in this cluster.  */
  const char *header;

  /* Used to order clusters within the same group with the same parent,
     according to the order in which they occurred in the parent argp's child
     list.  */
  int index;

  /* How to sort this cluster with respect to options and other clusters at the
     same depth (clusters always follow options in the same group).  */
  int group;

  /* The cluster to which this cluster belongs, or 0 if it's at the base
     level.  */
  struct hol_cluster *parent;

  /* The argp from which this cluster is (eventually) derived.  */
  const struct argp *argp;

  /* The distance this cluster is from the root.  */
  int depth;

  /* Clusters in a given hol are kept in a linked list, to make freeing them
     possible.  */
  struct hol_cluster *next;
};

/* A list of options for help.  */
struct hol
{
  /* An array of hol_entry's.  */
  struct hol_entry *entries;
  /* The number of entries in this hol.  If this field is zero, the others
     are undefined.  */
  unsigned num_entries;

  /* A string containing all short options in this HOL.  Each entry contains
     pointers into this string, so the order can't be messed with blindly.  */
  char *short_options;

  /* Clusters of entries in this hol.  */
  struct hol_cluster *clusters;
};

/* Create a struct hol from the options in ARGP.  CLUSTER is the
   hol_cluster in which these entries occur, or 0, if at the root.  */
static struct hol *
make_hol (const struct argp *argp, struct hol_cluster *cluster)
{
  char *so;
  const struct argp_option *o;
  const struct argp_option *opts = argp->options;
  struct hol_entry *entry;
  unsigned num_short_options = 0;
  struct hol *hol = malloc (sizeof (struct hol));

  hol->num_entries = 0;
  hol->clusters = 0;

  if (opts)
    {
      int cur_group = 0;

      /* The first option must not be an alias.  */

      /* Calculate the space needed.  */
      for (o = opts; ! oend (o); o++)
	{
	  if (! oalias (o))
	    hol->num_entries++;
	  if (oshort (o))
	    num_short_options++;	/* This is an upper bound.  */
	}

      hol->entries = malloc (sizeof (struct hol_entry) * hol->num_entries);
      hol->short_options = malloc (num_short_options + 1);

      /* Fill in the entries.  */
      so = hol->short_options;
      for (o = opts, entry = hol->entries; ! oend (o); entry++)
	{
	  entry->opt = o;
	  entry->num = 0;
	  entry->short_options = so;
	  entry->group = cur_group =
	    o->group
	    ? o->group
	    : ((!o->name && !o->key)
	       ? cur_group + 1
	       : cur_group);
	  entry->cluster = cluster;
	  entry->argp = argp;

	  do
	    {
	      entry->num++;
	      if (oshort (o) && ! find_char (o->key, hol->short_options, so))
		/* O has a valid short option which hasn't already been used.*/
		*so++ = o->key;
	      o++;
	    }
	  while (! oend (o) && oalias (o));
	}
      *so = '\0';		/* null terminated so we can find the length */
    }

  return hol;
}

/* Add a new cluster to HOL, with the given GROUP and HEADER (taken from the
   associated argp child list entry), INDEX, and PARENT, and return a pointer
   to it.  ARGP is the argp that this cluster results from.  */
static struct hol_cluster *
hol_add_cluster (struct hol *hol, int group, const char *header, int index,
		 struct hol_cluster *parent, const struct argp *argp)
{
  struct hol_cluster *cl = malloc (sizeof (struct hol_cluster));
  if (cl)
    {
      cl->group = group;
      cl->header = header;

      cl->index = index;
      cl->parent = parent;
      cl->argp = argp;
      cl->depth = parent ? parent->depth + 1 : 0;

      cl->next = hol->clusters;
      hol->clusters = cl;
    }
  return cl;
}

/* Free HOL and any resources it uses.  */
static void
hol_free (struct hol *hol)
{
  struct hol_cluster *cl = hol->clusters;

  while (cl)
    {
      struct hol_cluster *next = cl->next;
      free (cl);
      cl = next;
    }

  if (hol->num_entries > 0)
    {
      free (hol->entries);
      free (hol->short_options);
    }

  free (hol);
}

static int
hol_entry_short_iterate (const struct hol_entry *entry,
			 int (*func)(const struct argp_option *opt,
				     const struct argp_option *real,
				     const char *domain, void *cookie),
			 const char *domain, void *cookie)
{
  unsigned nopts;
  int val = 0;
  const struct argp_option *opt, *real = entry->opt;
  char *so = entry->short_options;

  for (opt = real, nopts = entry->num; nopts > 0 && !val; opt++, nopts--)
    if (oshort (opt) && *so == opt->key)
      {
	if (!oalias (opt))
	  real = opt;
	if (ovisible (opt))
	  val = (*func)(opt, real, domain, cookie);
	so++;
      }

  return val;
}

static int hol_entry_long_iterate (const struct hol_entry *entry,
			int (*func)(const struct argp_option *opt,
				    const struct argp_option *real,
				    const char *domain, void *cookie),
			const char *domain, void *cookie)
{
  unsigned nopts;
  int val = 0;
  const struct argp_option *opt, *real = entry->opt;

  for (opt = real, nopts = entry->num; nopts > 0 && !val; opt++, nopts--)
    if (opt->name)
      {
	if (!oalias (opt))
	  real = opt;
	if (ovisible (opt))
	  val = (*func)(opt, real, domain, cookie);
      }

  return val;
}

/* Iterator that returns true for the first short option.  */
static int
until_short (const struct argp_option *opt, const struct argp_option *real,
	     const char *domain, void *cookie)
{
  return oshort (opt) ? opt->key : 0;
}

/* Returns the first valid short option in ENTRY, or 0 if there is none.  */
static char
hol_entry_first_short (const struct hol_entry *entry)
{
  return hol_entry_short_iterate (entry, until_short,entry->argp->argp_domain, 0);
}

/* Returns the first valid long option in ENTRY, or 0 if there is none.  */
static const char *
hol_entry_first_long (const struct hol_entry *entry)
{
  const struct argp_option *opt;
  unsigned num;
  for (opt = entry->opt, num = entry->num; num > 0; opt++, num--)
    if (opt->name && ovisible (opt))
      return opt->name;
  return 0;
}

/* Returns the entry in HOL with the long option name NAME, or 0 if there is
   none.  */
static struct hol_entry *
hol_find_entry (struct hol *hol, const char *name)
{
  struct hol_entry *entry = hol->entries;
  unsigned num_entries = hol->num_entries;

  while (num_entries-- > 0)
    {
      const struct argp_option *opt = entry->opt;
      unsigned num_opts = entry->num;

      while (num_opts-- > 0)
	if (opt->name && ovisible (opt) && strcmp (opt->name, name) == 0)
	  return entry;
	else
	  opt++;

      entry++;
    }

  return 0;
}

/* If an entry with the long option NAME occurs in HOL, set it's special
   sort position to GROUP.  */
static void
hol_set_group (struct hol *hol, const char *name, int group)
{
  struct hol_entry *entry = hol_find_entry (hol, name);
  if (entry)
    entry->group = group;
}

/* Order by group:  0, 1, 2, ..., n, -m, ..., -2, -1.
   EQ is what to return if GROUP1 and GROUP2 are the same.  */
static int
group_cmp (int group1, int group2, int eq)
{
  if (group1 == group2)
    return eq;
  else if ((group1 < 0 && group2 < 0) || (group1 >= 0 && group2 >= 0))
    return group1 - group2;
  else
    return group2 - group1;
}

/* Compare clusters CL1 & CL2 by the order that they should appear in
   output.  */
static int
hol_cluster_cmp (const struct hol_cluster *cl1, const struct hol_cluster *cl2)
{
  /* If one cluster is deeper than the other, use its ancestor at the same
     level, so that finding the common ancestor is straightforward.  */
  while (cl1->depth < cl2->depth)
    cl1 = cl1->parent;
  while (cl2->depth < cl1->depth)
    cl2 = cl2->parent;

  /* Now reduce both clusters to their ancestors at the point where both have
     a common parent; these can be directly compared.  */
  while (cl1->parent != cl2->parent)
    cl1 = cl1->parent, cl2 = cl2->parent;

  return group_cmp (cl1->group, cl2->group, cl2->index - cl1->index);
}

/* Return the ancestor of CL that's just below the root (i.e., has a parent
   of 0).  */
static struct hol_cluster *
hol_cluster_base (struct hol_cluster *cl)
{
  while (cl->parent)
    cl = cl->parent;
  return cl;
}

/* Return true if CL1 is a child of CL2.  */
static int
hol_cluster_is_child (const struct hol_cluster *cl1,
		      const struct hol_cluster *cl2)
{
  while (cl1 && cl1 != cl2)
    cl1 = cl1->parent;
  return cl1 == cl2;
}

/* Given the name of a OPTION_DOC option, modifies NAME to start at the tail
   that should be used for comparisons, and returns true iff it should be
   treated as a non-option.  */
static int
canon_doc_option (const char **name)
{
  int non_opt;
  /* Skip initial whitespace.  */
  while (isspace (**name))
    (*name)++;
  /* Decide whether this looks like an option (leading `-') or not.  */
  non_opt = (**name != '-');
  /* Skip until part of name used for sorting.  */
  while (**name && !isalnum (**name))
    (*name)++;
  return non_opt;
}

/* Order ENTRY1 & ENTRY2 by the order which they should appear in a help
   listing.  */
static int
hol_entry_cmp (const struct hol_entry *entry1,
	       const struct hol_entry *entry2)
{
  /* The group numbers by which the entries should be ordered; if either is
     in a cluster, then this is just the group within the cluster.  */
  int group1 = entry1->group, group2 = entry2->group;

  if (entry1->cluster != entry2->cluster)
    {
      /* The entries are not within the same cluster, so we can't compare them
	 directly, we have to use the appropiate clustering level too.  */
      if (! entry1->cluster)
	/* ENTRY1 is at the `base level', not in a cluster, so we have to
	   compare it's group number with that of the base cluster in which
	   ENTRY2 resides.  Note that if they're in the same group, the
	   clustered option always comes laster.  */
	return group_cmp (group1, hol_cluster_base (entry2->cluster)->group, -1);
      else if (! entry2->cluster)
	/* Likewise, but ENTRY2's not in a cluster.  */
	return group_cmp (hol_cluster_base (entry1->cluster)->group, group2, 1);
      else
	/* Both entries are in clusters, we can just compare the clusters.  */
	return hol_cluster_cmp (entry1->cluster, entry2->cluster);
    }
  else if (group1 == group2)
    /* The entries are both in the same cluster and group, so compare them
       alphabetically.  */
    {
      int short1 = hol_entry_first_short (entry1);
      int short2 = hol_entry_first_short (entry2);
      int doc1 = odoc (entry1->opt);
      int doc2 = odoc (entry2->opt);
      const char *long1 = hol_entry_first_long (entry1);
      const char *long2 = hol_entry_first_long (entry2);

      if (doc1)
	doc1 = long1 != NULL && canon_doc_option (&long1);
      if (doc2)
	doc2 = long2 != NULL && canon_doc_option (&long2);

      if (doc1 != doc2)
	/* `documentation' options always follow normal options (or
	   documentation options that *look* like normal options).  */
	return doc1 - doc2;
      else if (!short1 && !short2 && long1 && long2)
	/* Only long options.  */
	return _stricmp (long1, long2);
      else
	/* Compare short/short, long/short, short/long, using the first
	   character of long options.  Entries without *any* valid
	   options (such as options with OPTION_HIDDEN set) will be put
	   first, but as they're not displayed, it doesn't matter where
	   they are.  */
	{
	  char first1 = short1 ? short1 : long1 ? *long1 : 0;
	  char first2 = short2 ? short2 : long2 ? *long2 : 0;
#ifdef _tolower
	  int lower_cmp = _tolower (first1) - _tolower (first2);
#else
	  int lower_cmp = tolower (first1) - tolower (first2);
#endif
	  /* Compare ignoring case, except when the options are both the
	     same letter, in which case lower-case always comes first.  */
	  return lower_cmp ? lower_cmp : first2 - first1;
	}
    }
  else
    /* Within the same cluster, but not the same group, so just compare
       groups.  */
    return group_cmp (group1, group2, 0);
}

/* Version of hol_entry_cmp with correct signature for qsort.  */
static int
hol_entry_qcmp (const void *entry1_v, const void *entry2_v)
{
  return hol_entry_cmp (entry1_v, entry2_v);
}

/* Sort HOL by group and alphabetically by option name (with short options
   taking precedence over long).  Since the sorting is for display purposes
   only, the shadowing of options isn't effected.  */
static void
hol_sort (struct hol *hol)
{
  if (hol->num_entries > 0)
    qsort (hol->entries, hol->num_entries, sizeof (struct hol_entry),
	   hol_entry_qcmp);
}

/* Append MORE to HOL, destroying MORE in the process.  Options in HOL shadow
   any in MORE with the same name.  */
static void
hol_append (struct hol *hol, struct hol *more)
{
  struct hol_cluster **cl_end = &hol->clusters;

  /* Steal MORE's cluster list, and add it to the end of HOL's.  */
  while (*cl_end)
    cl_end = &(*cl_end)->next;
  *cl_end = more->clusters;
  more->clusters = 0;

  /* Merge entries.  */
  if (more->num_entries > 0)
    {
      if (hol->num_entries == 0)
	{
	  hol->num_entries = more->num_entries;
	  hol->entries = more->entries;
	  hol->short_options = more->short_options;
	  more->num_entries = 0;	/* Mark MORE's fields as invalid.  */
	}
      else
	/* Append the entries in MORE to those in HOL, taking care to only add
	   non-shadowed SHORT_OPTIONS values.  */
	{
	  unsigned left;
	  char *so, *more_so;
	  struct hol_entry *e;
	  unsigned num_entries = hol->num_entries + more->num_entries;
	  struct hol_entry *entries =
	    malloc (num_entries * sizeof (struct hol_entry));
	  unsigned hol_so_len = strlen (hol->short_options);
	  char *short_options =
	    malloc (hol_so_len + strlen (more->short_options) + 1);

      memcpy (entries, hol->entries,
				hol->num_entries * sizeof (struct hol_entry));
	  memcpy (&entries[hol->num_entries],
		     more->entries,
		     more->num_entries * sizeof (struct hol_entry));

	  memcpy (short_options, hol->short_options, hol_so_len);

	  /* Fix up the short options pointers from HOL.  */
	  for (e = entries, left = hol->num_entries; left > 0; e++, left--)
	    e->short_options += (short_options - hol->short_options);

	  /* Now add the short options from MORE, fixing up its entries
	     too.  */
	  so = short_options + hol_so_len;
	  more_so = more->short_options;
	  for (left = more->num_entries; left > 0; e++, left--)
	    {
	      int opts_left;
	      const struct argp_option *opt;

	      e->short_options = so;

	      for (opts_left = e->num, opt = e->opt; opts_left; opt++, opts_left--)
		{
		  int ch = *more_so;
		  if (oshort (opt) && ch == opt->key)
		    /* The next short option in MORE_SO, CH, is from OPT.  */
		    {
		      if (! find_char (ch, short_options,
				       short_options + hol_so_len))
			/* The short option CH isn't shadowed by HOL's options,
			   so add it to the sum.  */
			*so++ = ch;
		      more_so++;
		    }
		}
	    }

	  *so = '\0';

	  free (hol->entries);
	  free (hol->short_options);

	  hol->entries = entries;
	  hol->num_entries = num_entries;
	  hol->short_options = short_options;
	}
    }

  hol_free (more);
}

/* Inserts enough spaces to make sure STREAM is at column COL.  */
static void
indent_to (argp_fmtstream_t stream, unsigned col)
{
  int needed = col - argp_fmtstream_point (stream);
  while (needed-- > 0)
    argp_fmtstream_putc (stream, ' ');
}

/* Output to STREAM either a space, or a newline if there isn't room for at
   least ENSURE characters before the right margin.  */
static void
space (argp_fmtstream_t stream, size_t ensure)
{
  if (argp_fmtstream_point (stream) + ensure
      >= argp_fmtstream_rmargin (stream))
    argp_fmtstream_putc (stream, '\n');
  else
    argp_fmtstream_putc (stream, ' ');
}

/* If the option REAL has an argument, we print it in using the printf
   format REQ_FMT or OPT_FMT depending on whether it's a required or
   optional argument.  */
static void
arg (const struct argp_option *real, const char *req_fmt, const char *opt_fmt,
     const char *domain, argp_fmtstream_t stream)
{
  if (real->arg)
    {
      if (real->flags & OPTION_ARG_OPTIONAL)
	argp_fmtstream_printf (stream, opt_fmt,real->arg);
      else
	argp_fmtstream_printf (stream, req_fmt,real->arg);
    }
}

/* Helper functions for hol_entry_help.  */

/* State used during the execution of hol_help.  */
struct hol_help_state
{
  /* PREV_ENTRY should contain the previous entry printed, or 0.  */
  struct hol_entry *prev_entry;

  /* If an entry is in a different group from the previous one, and SEP_GROUPS
     is true, then a blank line will be printed before any output. */
  int sep_groups;

  /* True if a duplicate option argument was suppressed (only ever set if
     UPARAMS.dup_args is false).  */
  int suppressed_dup_arg;
};

/* Some state used while printing a help entry (used to communicate with
   helper functions).  See the doc for hol_entry_help for more info, as most
   of the fields are copied from its arguments.  */
struct pentry_state
{
  const struct hol_entry *entry;
  argp_fmtstream_t stream;
  struct hol_help_state *hhstate;

  /* True if nothing's been printed so far.  */
  int first;

  /* If non-zero, the state that was used to print this help.  */
  const struct argp_state *state;
};

/* If a user doc filter should be applied to DOC, do so.  */
static const char *
filter_doc (const char *doc, int key, const struct argp *argp,
	    const struct argp_state *state)
{
  if (argp->help_filter)
    /* We must apply a user filter to this output.  */
    {
      void *input = argp_input (argp, state);
      return (*argp->help_filter) (key, doc, input);
    }
  else
    /* No filter.  */
    return doc;
}

/* Prints STR as a header line, with the margin lines set appropiately, and
   notes the fact that groups should be separated with a blank line.  ARGP is
   the argp that should dictate any user doc filtering to take place.  Note
   that the previous wrap margin isn't restored, but the left margin is reset
   to 0.  */
static void
print_header (const char *str, const struct argp *argp,
	      struct pentry_state *pest)
{
  const char *tstr = str;
  const char *fstr = filter_doc (tstr, ARGP_KEY_HELP_HEADER, argp, pest->state);

  if (fstr)
    {
      if (*fstr)
	{
	  if (pest->hhstate->prev_entry)
	    /* Precede with a blank line.  */
	    argp_fmtstream_putc (pest->stream, '\n');
	  indent_to (pest->stream, uparams.header_col);
	  argp_fmtstream_set_lmargin (pest->stream, uparams.header_col);
	  argp_fmtstream_set_wmargin (pest->stream, uparams.header_col);
	  argp_fmtstream_puts (pest->stream, fstr);
	  argp_fmtstream_set_lmargin (pest->stream, 0);
	  argp_fmtstream_putc (pest->stream, '\n');
	}

      pest->hhstate->sep_groups = 1; /* Separate subsequent groups. */
    }

  if (fstr != tstr)
    free ((char *) fstr);
}

/* Inserts a comma if this isn't the first item on the line, and then makes
   sure we're at least to column COL.  If this *is* the first item on a line,
   prints any pending whitespace/headers that should precede this line. Also
   clears FIRST.  */
static void
comma (unsigned col, struct pentry_state *pest)
{
  if (pest->first)
    {
      const struct hol_entry *pe = pest->hhstate->prev_entry;
      const struct hol_cluster *cl = pest->entry->cluster;

      if (pest->hhstate->sep_groups && pe && pest->entry->group != pe->group)
	argp_fmtstream_putc (pest->stream, '\n');

      if (cl && cl->header && *cl->header
	  && (!pe
	      || (pe->cluster != cl
		  && !hol_cluster_is_child (pe->cluster, cl))))
	/* If we're changing clusters, then this must be the start of the
	   ENTRY's cluster unless that is an ancestor of the previous one
	   (in which case we had just popped into a sub-cluster for a bit).
	   If so, then print the cluster's header line.  */
	{
	  int old_wm = argp_fmtstream_wmargin (pest->stream);
	  print_header (cl->header, cl->argp, pest);
	  argp_fmtstream_set_wmargin (pest->stream, old_wm);
	}

      pest->first = 0;
    }
  else
    argp_fmtstream_puts (pest->stream, ", ");

  indent_to (pest->stream, col);
}

/* Print help for ENTRY to STREAM.  */
static void
hol_entry_help (struct hol_entry *entry, const struct argp_state *state,
		argp_fmtstream_t stream, struct hol_help_state *hhstate)
{
  unsigned num;
  const struct argp_option *real = entry->opt, *opt;
  char *so = entry->short_options;
  int have_long_opt = 0;	/* We have any long options.  */
  /* Saved margins.  */
  int old_lm = argp_fmtstream_set_lmargin (stream, 0);
  int old_wm = argp_fmtstream_wmargin (stream);
  /* PEST is a state block holding some of our variables that we'd like to
     share with helper functions.  */
  struct pentry_state pest = { entry, stream, hhstate, 1, state };

  if (! odoc (real))
    for (opt = real, num = entry->num; num > 0; opt++, num--)
      if (opt->name && ovisible (opt))
	{
	  have_long_opt = 1;
	  break;
	}

  /* First emit short options.  */
  argp_fmtstream_set_wmargin (stream, uparams.short_opt_col); /* For truly bizarre cases. */
  for (opt = real, num = entry->num; num > 0; opt++, num--)
    if (oshort (opt) && opt->key == *so)
      /* OPT has a valid (non shadowed) short option.  */
      {
	if (ovisible (opt))
	  {
	    comma (uparams.short_opt_col, &pest);
	    argp_fmtstream_putc (stream, '-');
	    argp_fmtstream_putc (stream, *so);
	    if (!have_long_opt || uparams.dup_args)
	      arg (real, " %s", "[%s]",
		   state == NULL ? NULL : state->root_argp->argp_domain,
		   stream);
	    else if (real->arg)
	      hhstate->suppressed_dup_arg = 1;
	  }
	so++;
      }

  /* Now, long options.  */
  if (odoc (real))
    /* A `documentation' option.  */
    {
      argp_fmtstream_set_wmargin (stream, uparams.doc_opt_col);
      for (opt = real, num = entry->num; num > 0; opt++, num--)
	if (opt->name && ovisible (opt))
	  {
	    comma (uparams.doc_opt_col, &pest);
	    /* Calling gettext here isn't quite right, since sorting will
	       have been done on the original; but documentation options
	       should be pretty rare anyway...  */
	    argp_fmtstream_puts (stream,
				   opt->name);
	  }
    }
  else
    /* A real long option.  */
    {
      argp_fmtstream_set_wmargin (stream, uparams.long_opt_col);
      for (opt = real, num = entry->num; num > 0; opt++, num--)
	if (opt->name && ovisible (opt))
	  {
	    comma (uparams.long_opt_col, &pest);
	    argp_fmtstream_printf (stream, "--%s", opt->name);
	    arg (real, "=%s", "[=%s]",
		 state == NULL ? NULL : state->root_argp->argp_domain, stream);
	  }
    }

  /* Next, documentation strings.  */
  argp_fmtstream_set_lmargin (stream, 0);

  if (pest.first)
    {
      /* Didn't print any switches, what's up?  */
      if (!oshort (real) && !real->name)
	/* This is a group header, print it nicely.  */
	print_header (real->doc, entry->argp, &pest);
      else
	/* Just a totally shadowed option or null header; print nothing.  */
	goto cleanup;		/* Just return, after cleaning up.  */
    }
  else
    {
      const char *tstr = real->doc ? real->doc : 0;
      const char *fstr = filter_doc (tstr, real->key, entry->argp, state);
      if (fstr && *fstr)
	{
	  unsigned int col = argp_fmtstream_point (stream);

	  argp_fmtstream_set_lmargin (stream, uparams.opt_doc_col);
	  argp_fmtstream_set_wmargin (stream, uparams.opt_doc_col);

	  if (col > (unsigned int) (uparams.opt_doc_col + 3))
	    argp_fmtstream_putc (stream, '\n');
	  else if (col >= (unsigned int) uparams.opt_doc_col)
	    argp_fmtstream_puts (stream, "   ");
	  else
	    indent_to (stream, uparams.opt_doc_col);

	  argp_fmtstream_puts (stream, fstr);
	}
      if (fstr && fstr != tstr)
	free ((char *) fstr);

      /* Reset the left margin.  */
      argp_fmtstream_set_lmargin (stream, 0);
      argp_fmtstream_putc (stream, '\n');
    }

  hhstate->prev_entry = entry;

cleanup:
  argp_fmtstream_set_lmargin (stream, old_lm);
  argp_fmtstream_set_wmargin (stream, old_wm);
}

/* Output a long help message about the options in HOL to STREAM.  */
static void hol_help (struct hol *hol, const struct argp_state *state,
	  argp_fmtstream_t stream)
{
  unsigned num;
  struct hol_entry *entry;
  struct hol_help_state hhstate = { 0, 0, 0 };

  for (entry = hol->entries, num = hol->num_entries; num > 0; entry++, num--)
    hol_entry_help (entry, state, stream, &hhstate);

  if (hhstate.suppressed_dup_arg && uparams.dup_args_note)
    {
      const char *tstr = "Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options.";
      const char *fstr = filter_doc (tstr, ARGP_KEY_HELP_DUP_ARGS_NOTE,
				     state ? state->root_argp : 0, state);
      if (fstr && *fstr)
	{
	  argp_fmtstream_putc (stream, '\n');
	  argp_fmtstream_puts (stream, fstr);
	  argp_fmtstream_putc (stream, '\n');
	}
      if (fstr && fstr != tstr)
	free ((char *) fstr);
    }
}

/* Helper functions for hol_usage.  */

/* If OPT is a short option without an arg, append its key to the string
   pointer pointer to by COOKIE, and advance the pointer.  */
static int add_argless_short_opt (const struct argp_option *opt,
		       const struct argp_option *real,
		       const char *domain, void *cookie)
{
  char **snao_end = cookie;
  if (!(opt->arg || real->arg)
      && !((opt->flags | real->flags) & OPTION_NO_USAGE))
    *(*snao_end)++ = opt->key;
  return 0;
}

/* If OPT is a short option with an arg, output a usage entry for it to the
   stream pointed at by COOKIE.  */
static int usage_argful_short_opt (const struct argp_option *opt,
			const struct argp_option *real,
			const char *domain, void *cookie)
{
  argp_fmtstream_t stream = cookie;
  const char *arg = opt->arg;
  int flags = opt->flags | real->flags;

  if (! arg)
    arg = real->arg;

  if (arg && !(flags & OPTION_NO_USAGE))
    {
      arg = arg;

      if (flags & OPTION_ARG_OPTIONAL)
	argp_fmtstream_printf (stream, " [-%c[%s]]", opt->key, arg);
      else
	{
	  /* Manually do line wrapping so that it (probably) won't
	     get wrapped at the embedded space.  */
	  space (stream, 6 + strlen (arg));
	  argp_fmtstream_printf (stream, "[-%c %s]", opt->key, arg);
	}
    }

  return 0;
}

/* Output a usage entry for the long option opt to the stream pointed at by
   COOKIE.  */
static int usage_long_opt (const struct argp_option *opt,
		const struct argp_option *real,
		const char *domain, void *cookie)
{
  argp_fmtstream_t stream = cookie;
  const char *arg = opt->arg;
  int flags = opt->flags | real->flags;

  if (! arg)
    arg = real->arg;

  if (! (flags & OPTION_NO_USAGE))
    {
      if (arg)
	{
	  arg = arg;
	  if (flags & OPTION_ARG_OPTIONAL)
	    argp_fmtstream_printf (stream, " [--%s[=%s]]", opt->name, arg);
	  else
	    argp_fmtstream_printf (stream, " [--%s=%s]", opt->name, arg);
	}
      else
	argp_fmtstream_printf (stream, " [--%s]", opt->name);
    }

  return 0;
}

/* Print a short usage description for the arguments in HOL to STREAM.  */
static void hol_usage (struct hol *hol, argp_fmtstream_t stream)
{
  if (hol->num_entries > 0)
    {
      unsigned nentries;
      struct hol_entry *entry;
      char *short_no_arg_opts = malloc (strlen (hol->short_options) + 1);
      char *snao_end = short_no_arg_opts;

      /* First we put a list of short options without arguments.  */
      for (entry = hol->entries, nentries = hol->num_entries
	   ; nentries > 0
	   ; entry++, nentries--)
	hol_entry_short_iterate (entry, add_argless_short_opt,
				 entry->argp->argp_domain, &snao_end);
      if (snao_end > short_no_arg_opts)
	{
	  *snao_end++ = 0;
	  argp_fmtstream_printf (stream, " [-%s]", short_no_arg_opts);
	}

      /* Now a list of short options *with* arguments.  */
      for (entry = hol->entries, nentries = hol->num_entries
	   ; nentries > 0
	   ; entry++, nentries--)
	hol_entry_short_iterate (entry, usage_argful_short_opt,
				 entry->argp->argp_domain, stream);

      /* Finally, a list of long options (whew!).  */
      for (entry = hol->entries, nentries = hol->num_entries
	   ; nentries > 0
	   ; entry++, nentries--)
	hol_entry_long_iterate (entry, usage_long_opt,
				entry->argp->argp_domain, stream);
    }
}

/* Make a HOL containing all levels of options in ARGP.  CLUSTER is the
   cluster in which ARGP's entries should be clustered, or 0.  */
static struct hol *argp_hol (const struct argp *argp, struct hol_cluster *cluster)
{
  const struct argp_child *child = argp->children;
  struct hol *hol = make_hol (argp, cluster);
  if (child)
    while (child->argp)
      {
	struct hol_cluster *child_cluster =
	  ((child->group || child->header)
	   /* Put CHILD->argp within its own cluster.  */
	   ? hol_add_cluster (hol, child->group, child->header,
			      child - argp->children, cluster, argp)
	   /* Just merge it into the parent's cluster.  */
	   : cluster);
	hol_append (hol, argp_hol (child->argp, child_cluster)) ;
	child++;
      }
  return hol;
}

/* Calculate how many different levels with alternative args strings exist in
   ARGP.  */
static size_t argp_args_levels (const struct argp *argp)
{
  size_t levels = 0;
  const struct argp_child *child = argp->children;

  if (argp->args_doc && strchr (argp->args_doc, '\n'))
    levels++;

  if (child)
    while (child->argp)
      levels += argp_args_levels ((child++)->argp);

  return levels;
}

/* Print all the non-option args documented in ARGP to STREAM.  Any output is
   preceded by a space.  LEVELS is a pointer to a byte vector the length
   returned by argp_args_levels; it should be initialized to zero, and
   updated by this routine for the next call if ADVANCE is true.  True is
   returned as long as there are more patterns to output.  */
static int
argp_args_usage (const struct argp *argp, const struct argp_state *state,
		 char **levels, int advance, argp_fmtstream_t stream)
{
  char *our_level = *levels;
  int multiple = 0;
  const struct argp_child *child = argp->children;
  const char *tdoc = argp->args_doc, *nl = 0;
  const char *fdoc = filter_doc (tdoc, ARGP_KEY_HELP_ARGS_DOC, argp, state);

  if (fdoc)
    {
      const char *cp = fdoc;
      nl = __strchrnul (cp, '\n');
      if (*nl != '\0')
	/* This is a `multi-level' args doc; advance to the correct position
	   as determined by our state in LEVELS, and update LEVELS.  */
	{
	  int i;
	  multiple = 1;
	  for (i = 0; i < *our_level; i++)
	    cp = nl + 1, nl = __strchrnul (cp, '\n');
	  (*levels)++;
	}

      /* Manually do line wrapping so that it (probably) won't get wrapped at
	 any embedded spaces.  */
      space (stream, 1 + nl - cp);

      argp_fmtstream_write (stream, cp, nl - cp);
    }
  if (fdoc && fdoc != tdoc)
    free ((char *)fdoc);	/* Free user's modified doc string.  */

  if (child)
    while (child->argp)
      advance = !argp_args_usage ((child++)->argp, state, levels, advance, stream);

  if (advance && multiple)
    {
      /* Need to increment our level.  */
      if (*nl)
	/* There's more we can do here.  */
	{
	  (*our_level)++;
	  advance = 0;		/* Our parent shouldn't advance also. */
	}
      else if (*our_level > 0)
	/* We had multiple levels, but used them up; reset to zero.  */
	*our_level = 0;
    }

  return !advance;
}

/* Print the documentation for ARGP to STREAM; if POST is false, then
   everything preceeding a `\v' character in the documentation strings (or
   the whole string, for those with none) is printed, otherwise, everything
   following the `\v' character (nothing for strings without).  Each separate
   bit of documentation is separated a blank line, and if PRE_BLANK is true,
   then the first is as well.  If FIRST_ONLY is true, only the first
   occurrence is output.  Returns true if anything was output.  */
static int argp_doc (const struct argp *argp, const struct argp_state *state,
	  int post, int pre_blank, int first_only,argp_fmtstream_t stream)
{
  const char *text;
  const char *inp_text;
  void *input = 0;
  int anything = 0;
  size_t inp_text_limit = 0;
  const char *doc = argp->doc;
  const struct argp_child *child = argp->children;

  if (doc)
    {
      char *vt = strchr (doc, '\v');
      inp_text = post ? (vt ? vt + 1 : 0) : doc;
      inp_text_limit = (!post && vt) ? (vt - doc) : 0;
    }
  else
    inp_text = 0;

  if (argp->help_filter)
    /* We have to filter the doc strings.  */
    {
      if (inp_text_limit)
	/* Copy INP_TEXT so that it's nul-terminated.  */
	inp_text = strndup (inp_text, inp_text_limit);
      input = argp_input (argp, state);
      text =
	(*argp->help_filter) (post
			      ? ARGP_KEY_HELP_POST_DOC
			      : ARGP_KEY_HELP_PRE_DOC,
			      inp_text, input);
    }
  else
    text = (const char *) inp_text;

  if (text)
    {
      if (pre_blank)
	argp_fmtstream_putc (stream, '\n');

      if (text == inp_text && inp_text_limit)
	argp_fmtstream_write (stream, inp_text, inp_text_limit);
      else
	argp_fmtstream_puts (stream, text);

      if (argp_fmtstream_point (stream) > argp_fmtstream_lmargin (stream))
	argp_fmtstream_putc (stream, '\n');

      anything = 1;
    }

  if (text && text != inp_text)
    free ((char *) text);	/* Free TEXT returned from the help filter.  */
  if (inp_text && inp_text_limit && argp->help_filter)
    free ((char *) inp_text);	/* We copied INP_TEXT, so free it now.  */

  if (post && argp->help_filter)
    /* Now see if we have to output a ARGP_KEY_HELP_EXTRA text.  */
    {
      text = (*argp->help_filter) (ARGP_KEY_HELP_EXTRA, 0, input);
      if (text)
	{
	  if (anything || pre_blank)
	    argp_fmtstream_putc (stream, '\n');
	  argp_fmtstream_puts (stream, text);
	  free ((char *) text);
	  if (argp_fmtstream_point (stream)
	      > argp_fmtstream_lmargin (stream))
	    argp_fmtstream_putc (stream, '\n');
	  anything = 1;
	}
    }

  if (child)
    while (child->argp && !(first_only && anything))
      anything |=
	argp_doc ((child++)->argp, state,
		  post, anything || pre_blank, first_only,
		  stream);

  return anything;
}

/* Output a usage message for ARGP to STREAM.  If called from
   argp_state_help, STATE is the relevent parsing state.  FLAGS are from the
   set ARGP_HELP_*.  NAME is what to use wherever a `program name' is
   needed. */
static void _help (const struct argp *argp, const struct argp_state *state, FILE *stream,unsigned flags, char *name)
{
  int anything = 0;		/* Whether we've output anything.  */
  struct hol *hol = 0;
  argp_fmtstream_t fs;

  if (! stream)
    return;

  fill_in_uparams (state);

  fs = argp_make_fmtstream (stream, 0, uparams.rmargin, 0);
  if (! fs)
    {
      return;
    }

  if (flags & (ARGP_HELP_USAGE | ARGP_HELP_SHORT_USAGE | ARGP_HELP_LONG))
    {
      hol = argp_hol (argp, 0);

      /* If present, these options always come last.  */
      hol_set_group (hol, "help", -1);
      hol_set_group (hol, "version", -1);

      hol_sort (hol);
    }

  if (flags & (ARGP_HELP_USAGE | ARGP_HELP_SHORT_USAGE))
    /* Print a short `Usage:' message.  */
    {
      int first_pattern = 1, more_patterns;
      size_t num_pattern_levels = argp_args_levels (argp);
      char *pattern_levels = malloc (num_pattern_levels);

      memset (pattern_levels, 0, num_pattern_levels);

      do
	{
	  int old_lm;
	  int old_wm = argp_fmtstream_set_wmargin (fs, uparams.usage_indent);
	  char *levels = pattern_levels;

	  if (first_pattern)
	    argp_fmtstream_printf (fs, "%s %s",
				     "Usage:",
				     name);
	  else
	    argp_fmtstream_printf (fs, "%s %s",
				     "  or: ",
				     name);

	  /* We set the lmargin as well as the wmargin, because hol_usage
	     manually wraps options with newline to avoid annoying breaks.  */
	  old_lm = argp_fmtstream_set_lmargin (fs, uparams.usage_indent);

	  if (flags & ARGP_HELP_SHORT_USAGE)
	    /* Just show where the options go.  */
	    {
	      if (hol->num_entries > 0)
		argp_fmtstream_puts (fs, " [OPTION...]");
	    }
	  else
	    /* Actually print the options.  */
	    {
	      hol_usage (hol, fs);
	      flags |= ARGP_HELP_SHORT_USAGE; /* But only do so once.  */
	    }

	  more_patterns = argp_args_usage (argp, state, &levels, 1, fs);

	  argp_fmtstream_set_wmargin (fs, old_wm);
	  argp_fmtstream_set_lmargin (fs, old_lm);

	  argp_fmtstream_putc (fs, '\n');
	  anything = 1;

	  first_pattern = 0;
	}
      while (more_patterns);
    }

  if (flags & ARGP_HELP_PRE_DOC)
    anything |= argp_doc (argp, state, 0, 0, 1, fs);

  if (flags & ARGP_HELP_SEE)
    {
      argp_fmtstream_printf (fs,"Try `%s --help' or `%s --usage' for more information.\n",
			       name, name);
      anything = 1;
    }

  if (flags & ARGP_HELP_LONG)
    /* Print a long, detailed help message.  */
    {
      /* Print info about all the options.  */
      if (hol->num_entries > 0)
	{
	  if (anything)
	    argp_fmtstream_putc (fs, '\n');
	  hol_help (hol, state, fs);
	  anything = 1;
	}
    }

  if (flags & ARGP_HELP_POST_DOC)
    /* Print any documentation strings at the end.  */
    anything |= argp_doc (argp, state, 1, anything, 0, fs);

  if ((flags & ARGP_HELP_BUG_ADDR) && argp_program_bug_address)
    {
      if (anything)
	argp_fmtstream_putc (fs, '\n');
      argp_fmtstream_printf (fs,argp->argp_domain,
					     "Report bugs to %s.\n",
 			       argp_program_bug_address);
      anything = 1;
    }

  if (hol)
    hol_free (hol);

  argp_fmtstream_free (fs);
}

/* Output a usage message for ARGP to STREAM.  FLAGS are from the set
   ARGP_HELP_*.  NAME is what to use wherever a `program name' is needed. */
void argp_help (const struct argp *argp, FILE *stream,
		  unsigned flags, char *name)
{
  _help (argp, 0, stream, flags, name);
}

/* Output, if appropriate, a usage message for STATE to STREAM.  FLAGS are
   from the set ARGP_HELP_*.  */
void argp_state_help (const struct argp_state *state, FILE *stream, unsigned flags)
{
  if ((!state || ! (state->flags & ARGP_NO_ERRS)) && stream)
    {
      if (state && (state->flags & ARGP_LONG_ONLY))
	flags |= ARGP_HELP_LONG_ONLY;

      _help (state ? state->root_argp : 0, state, stream, flags,
	     state ? state->name : "");

      if (!state || ! (state->flags & ARGP_NO_EXIT))
	{
	  if (flags & ARGP_HELP_EXIT_ERR)
	    exit (argp_err_exit_status);
	  if (flags & ARGP_HELP_EXIT_OK)
	    exit (0);
	}
  }
}

/* If appropriate, print the printf string FMT and following args, preceded
   by the program name and `:', to stderr, and followed by a `Try ... --help'
   message, then exit (1).  */
void argp_error (const struct argp_state *state, const char *fmt, ...)
{
  if (!state || !(state->flags & ARGP_NO_ERRS))
    {
      FILE *stream = state ? state->err_stream : stderr;

      if (stream)
	{
	  va_list ap;

	  va_start (ap, fmt);


	  fputs (state ? state->name : "",
			  stream);
	  putc (':', stream);
	  putc (' ', stream);

	  vfprintf (stream, fmt, ap);

	  putc ('\n', stream);

	  argp_state_help (state, stream, ARGP_HELP_STD_ERR);

	  va_end (ap);

	}
    }
}

/* Similar to the standard gnu error-reporting function error(), but will
   respect the ARGP_NO_EXIT and ARGP_NO_ERRS flags in STATE, and will print
   to STATE->err_stream.  This is useful for argument parsing code that is
   shared between program startup (when exiting is desired) and runtime
   option parsing (when typically an error code is returned instead).  The
   difference between this function and argp_error is that the latter is for
   *parsing errors*, and the former is for other problems that occur during
   parsing but don't reflect a (syntactic) problem with the input.  */
void argp_failure (const struct argp_state *state, int status, int errnum,
		const char *fmt, ...)
{
  if (!state || !(state->flags & ARGP_NO_ERRS))
    {
      FILE *stream = state ? state->err_stream : stderr;

      if (stream)
	{
	  fputs (state ? state->name : "",
			  stream);

	  if (fmt)
	    {
	      va_list ap;

	      va_start (ap, fmt);

	      putc (':', stream);
	      putc (' ', stream);

	      vfprintf (stream, fmt, ap);

	      va_end (ap);
	    }

	  if (errnum)
	    {
	      putc (':', stream);
	      putc (' ', stream);
//	      fputs (__strerror_r (errnum, buf, sizeof (buf)), stream);
	      fputs (strerror (errnum), stream);
	    }

	    putc ('\n', stream);

	  if (status && (!state || !(state->flags & ARGP_NO_EXIT)))
	    exit (status);
	}
    }
}

void argp_usage(const  struct argp_state *state)
{
  argp_state_help(state,stderr,ARGP_HELP_STD_ERR);
}

#endif //HAVE_ARGP_H
