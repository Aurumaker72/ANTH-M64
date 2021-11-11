#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "M64.h"

MovieStatus Parse(FILE* FilePtr, SMovieHeader* HeaderPtr, SAMPLETYPE* InputsPtr)
{
	// first and foremost nuke the header, good luck if you dont account for this in your code
	memset(HeaderPtr, 0, sizeof(HeaderPtr));

	if (!HeaderPtr) return BADALLOCATION;

	// read movie header and convert it to new if needed
	int result = internalReadMovieHeader(FilePtr, HeaderPtr, 1);
	if (result != SUCCESS) return result; // on fail just return and done
	
	// now we read the inputs into array but first do some sanity checks for it to not explode
	if (HeaderPtr->Samples < 0) return SAMPLEOUTOFBOUNDS; // movie is less than 0 samples long, dont try to read

	// end of header in bytes
	int hEnd = HeaderPtr->Version == 3 ? MUP_HEADER_SIZE : MUP_HEADER_SIZE_OLD;

	fseek(FilePtr, 0, SEEK_END); // find eof
	long fLen = ftell(FilePtr);

	if ((fLen - hEnd) / sizeof(SAMPLETYPE) != HeaderPtr->Samples) // if the amount of samples (bytesamt/4) from eof to header end isnt equal to samples then just return. this would otherwise cause huge issues later
		return INVALIDFRAME;
	
	//SAMPLETYPE* samples = (SAMPLETYPE*)malloc(HeaderPtr->Samples); // idk how good it is to just call malloc, this might make debugging a bit harder. we should actually clear that out

	fseek(FilePtr, hEnd, SEEK_SET); // rewind to header end

	fread(InputsPtr, sizeof(SAMPLETYPE), HeaderPtr->Samples, FilePtr); // read all inputs

	//free(samples);
	
	return SUCCESS;

}

MovieStatus Parse2(FILE* FilePtr, SMovieHeader* HeaderPtr, SAMPLETYPE* InputsPtr)
{
	// first and foremost nuke the header, good luck if you dont account for this in your code
	memset(HeaderPtr, 0, sizeof(HeaderPtr));

	if (!HeaderPtr) return BADALLOCATION;

	// read movie header and convert it to new if needed
	int result = internalReadMovieHeader(FilePtr, HeaderPtr, 1);
	if (result != SUCCESS) return result; // on fail just return and done

	// now we read the inputs into array but first do some sanity checks for it to not explode
	if (HeaderPtr->Samples < 0) return SAMPLEOUTOFBOUNDS; // movie is less than 0 samples long, dont try to read

	// end of header in bytes
	int hEnd = HeaderPtr->Version == 3 ? MUP_HEADER_SIZE : MUP_HEADER_SIZE_OLD;

	fseek(FilePtr, 0, SEEK_END); // find eof
	long fLen = ftell(FilePtr);

	if ((fLen - hEnd) / sizeof(SAMPLETYPE) != HeaderPtr->Samples) // if the amount of samples (bytesamt/4) from eof to header end isnt equal to samples then just return. this would otherwise cause huge issues later
		return INVALIDFRAME;

	SAMPLETYPE* samples = (SAMPLETYPE*)malloc(sizeof(samples) * HeaderPtr->Samples); // idk how good it is to just call malloc, this might make debugging a bit harder. we should actually clear that out

	fseek(FilePtr, hEnd, SEEK_SET); // rewind to header end

	fread(InputsPtr, sizeof(SAMPLETYPE), sizeof(samples) * HeaderPtr->Samples, FilePtr); // read all inputs
	
	realloc(InputsPtr, sizeof(FilePtr));
	memcpy(InputsPtr, samples, sizeof(samples)*HeaderPtr->Samples); // oh vs you whiny piece of shit shut up about this being unsafe. it isnt

	free(samples); // is this safe? memcpy copies data deeply and doesnt just internally do pointer arithmetic so this is safe?

	return SUCCESS;

}

// do not call it manually
MovieStatus internalReadMovieHeader(FILE* file, SMovieHeader* header, int push)
{

	fseek(file, 0L, SEEK_SET);

	SMovieHeader newHeader;
	memset(&newHeader, 0, sizeof(SMovieHeader));

	if (fread(&newHeader, 1, MUP_HEADER_SIZE_OLD, file) != MUP_HEADER_SIZE_OLD)
		return UNKNOWNVERSION;

	if (newHeader.Magic != MUP_MAGIC)
		return WRONGMAGIC;

	if (newHeader.Version <= 0 || newHeader.Version > MUP_VERSION)
		return WRONGVERSION;

	if (newHeader.Version == 1 || newHeader.Version == 2)
	{
#define ISASCII(x) (((x) >= 'A' && (x) <= 'Z') || ((x) >= 'a' && (x) <= 'z') || ((x) == '1'))

		int i;
		for (i = 0; i < 56 + 64; i++)
			if (ISASCII(newHeader.ReservedBytes[i])
				&& ISASCII(newHeader.ReservedBytes[i + 64])
				&& ISASCII(newHeader.ReservedBytes[i + 64 + 64])
				&& ISASCII(newHeader.ReservedBytes[i + 64 + 64 + 64]))
				break;
		if (i != 56 + 64)
		{
			memmove(newHeader.VideoPluginName, newHeader.ReservedBytes + i, 256);
		}
		else
		{
			for (i = 0; i < 56 + 64; i++)
				if (ISASCII(newHeader.ReservedBytes[i])
					&& ISASCII(newHeader.ReservedBytes[i + 64])
					&& ISASCII(newHeader.ReservedBytes[i + 64 + 64]))
					break;
			if (i != 56 + 64)
				memmove(newHeader.SoundPluginName, newHeader.ReservedBytes + i, 256 - 64);
			else
			{
				for (i = 0; i < 56 + 64; i++)
					if (ISASCII(newHeader.ReservedBytes[i])
						&& ISASCII(newHeader.ReservedBytes[i + 64]))
						break;
				if (i != 56 + 64)
					memmove(newHeader.InputPluginName, newHeader.ReservedBytes + i, 256 - 64 - 64);
				else
				{
					for (i = 0; i < 56 + 64; i++)
						if (ISASCII(newHeader.ReservedBytes[i]))
							break;
					if (i != 56 + 64)
						memmove(newHeader.RSPPluginName, newHeader.ReservedBytes + i, 256 - 64 - 64 - 64);
					else
						strncpy(newHeader.RSPPluginName, UNKNOWN_TEXT, 64);

					strncpy(newHeader.InputPluginName, UNKNOWN_TEXT, 64);
				}
				strncpy(newHeader.SoundPluginName, UNKNOWN_TEXT, 64);
			}
			strncpy(newHeader.VideoPluginName, UNKNOWN_TEXT, 64);
		}
		strncpy(newHeader.AuthorInfo, newHeader.OldAuthorInfo, 48);
		strncpy(newHeader.Description, newHeader.OldDescription, 80);
	}
	if (newHeader.Version >= 3 && newHeader.Version <= MUP_VERSION)
	{
		if (fread((char*)(&newHeader) + MUP_HEADER_SIZE_OLD, 1, MUP_HEADER_SIZE - MUP_HEADER_SIZE_OLD, file) != MUP_HEADER_SIZE - MUP_HEADER_SIZE_OLD) // dump all bytes into struct and pray... also check if discrepancy of new->old isnt what its supposed to be
			return UNKNOWNVERSION;
	}

	*header = newHeader;

	return 1;
}
