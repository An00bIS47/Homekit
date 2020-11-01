/*  
  File:       BUILDNUMBER.C
  Homepage:   http://buildnumber.sourceforge.net

  See README.TXT for full documentation.

  BuildNumber is released under the BSD license. See LICENSE.TXT
  for the terms, a copy of which must be included with this source,
  even if the source is modified.

  �2006 John M. Stoneham. All rights reserved.
 */

#define MAJOR_VERSION 0
#define MINOR_VERSION 8

/* 
  A note on the following #define, though it's more of a rant. I
  understand the need for more secure versions of some of the C
  Standard Library functions which are notoriously problematic when
  used incorrectly, like strcpy() and scanf() and others. But
  declaring all such functions as DEPRECATED is the domain of the
  Standards Committee, not the implementation, period. The compiler
  SHOULD NOT spew out thousands of deprecation warnings for properly
  used Standard Library functions!!! It is understandable to offer
  more secure versions such as printf_s() and scanf_s(), but to
  declare the Standard Library functions as _DEPRECATED_ is non-
  conformant, nonstandard, and nonsensical; but this is exactly what
  Microsoft has done. The following is needed to disable all those 
  idiotic warnings, and since this  behavior will likely continue in
  future versions, it is set for all versions of the Microsoft 
  compiler.
 */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif /* _MSC_VER */

/* max and min build numbers, maximum 4 characters each */
#define MAX_BUILDS 9999
#define MIN_BUILDS -999

/* location of line "#define BUILDNUMBER" in buildnumber.h 
   If the format of the file changes, this needs to change too */
#define NUMBER_LINE 6 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Buildnumber itself makes use of a buildnumber.h */
#include "buildnumber.h"

int main(int argc, char* argv[])
{
	FILE *file;
	int i = 0;
	char line[80];
	size_t length = 0;
	int argCount = 1;

	char headerPath[255];

	strcpy(headerPath, "HAPBuildnumber.hpp");

	/* intially set to 0 */
	int build_number = 0;

	/* see if a command-line option was passed */
	if(argc > 1)
	{
		/* "--version" */
		if(strncmp(argv[argCount], "--version", 9) == 0)
		{
			printf("Buildnumber version %d.%d build " BUILDNUMBER_STR "\n",
					MAJOR_VERSION, MINOR_VERSION);
			return 0;
		}
		/* "--help" */
		else if(strncmp(argv[argCount], "--help", 6) == 0)
		{
			printf("Buildnumber version %d.%d build " BUILDNUMBER_STR "\n",
					MAJOR_VERSION, MINOR_VERSION);
			printf("Usage: buildnumber [OPTION]\n\n");
			printf("Options:\n");
			printf("  --set [num]  		create/update buildnumber using [num]\n");
			printf("  --header [file]  	create/update buildnumber using [file]\n");
			printf("  --version    		display current verion (and exit)\n");
			printf("  --help       		display this message (and exit)\n\n");
			return 0;
		}

		if(strncmp(argv[argCount], "--header", 8) == 0) {
			printf("Use custom header... \n");
			strcpy(headerPath, argv[argCount+1]);
			argCount = argCount + 1;
		}

		if(strncmp(argv[argCount], "--set", 5) == 0)
		{
			build_number = atoi(argv[argCount+1]);
			argCount = argCount + 1;
		}

		/* atoi will return 0 is this isn't a number, which is
       the behavior we want */


		/* keep the number in range*/
		if(build_number > MAX_BUILDS)
			build_number = MAX_BUILDS;
		else if(build_number < MIN_BUILDS)
			build_number = MIN_BUILDS;

		if(argc > 6)
			fprintf(stderr, "BuildNumber: extra command-line options ignored\n");
	}

	/* if there was no command-line option, or if it was invalid or 0,
     try to load the existing file */
	if(build_number == 0)
	{
		file = fopen(headerPath, "r");

		/* if it isn't there, this is the first time using BuildNumber here */
		if(file == NULL)
			build_number = 1;

		else /* buildnumber.h exists, so parse it */
		{
			/* we only want to keep the line located at NUMBER_LINE, skip the rest
         but stop if the file is too short */
			for(i = 0; i < NUMBER_LINE; i++)
			{
				if((fgets(line, 80, file) == NULL) || feof(file))
					break;
			}

			/* if we didn't finish the loop, something was wrong with the file */
			if(i < NUMBER_LINE)
				build_number = 1;

			else /* file OK so far */
			{
				/* the number itself begins at location line[30] and should never
           exceed 4 digits, so length should always be less than 25 */
				line[35] = '\0'; /* make sure it stops here, for safety */
				length = strlen(line);

				if(length >= 30 && length < 35)
				{
					/* if atoi() fails it returns 0, which will be incremented to 1 */
					build_number = atoi(&line[30]);

					if(build_number > MAX_BUILDS)
						build_number = MAX_BUILDS;

					else if(build_number < MIN_BUILDS)
						build_number = MIN_BUILDS;

					else if(build_number < MAX_BUILDS)
						build_number++;

					/* otherwise, build_number == MAX_BUILDS, so don't do anything */

				}
				else /* the line wasn't in the correct format */
					build_number = 1;
			}

			fclose(file);
		}
	}

	else /* using the value from the command-line */
		printf("BuildNumber: using command-line option as value\n");

	/* open the file for writing, creating it if necessary */
	file = fopen(headerPath, "w");

	if(file == NULL) /* oops, can't create it, return an error status */
	{
		fprintf(stderr, "BuildNumber: error creating HAPBuildnumber.hpp\n");
		return EXIT_FAILURE;
	}

	else
	{
		fprintf(file, "/* Generated by BuildNumber Version %d.%d */\n\n",
				MAJOR_VERSION, MINOR_VERSION);
		fprintf(file, "#ifndef HAPBUILDNUMBER_HPP_ \n");
		fprintf(file, "#define HAPBUILDNUMBER_HPP_ \n\n");
		fprintf(file, "#define HOMEKIT_VERSION_BUILD %d\n", build_number);
		fprintf(file, "#define HOMEKIT_VERSION_BUILD_STR \"%d\" \n\n", build_number);
		fprintf(file, "#endif /* HAPBUILDNUMBER_HPP_ */\n");

		fclose(file);

		printf("BuildNumber: new buildnumber is %d\n", build_number);
	}

	return 0;
}

