#pragma once

#define MUP_MAGIC (0x1a34364d)
#define MUP_VERSION (3) // TODO: dont hardcode it
#define MUP_HEADER_SIZE_OLD (512)
#define MUP_HEADER_SIZE (sizeof(SMovieHeader)) // watch it be 1 byte off lmfao
#define SAMPLETYPE int
#define UNKNOWN_TEXT "(unknown)"

typedef enum MovieStatus {
	SUCCESS = 0,
	UNKNOWNVERSION,        
	WRONGVERSION, 
	WRONGMAGIC,
	FILENOTFOUND,    
	INVALIDFRAME,    
	UNKNOWNERROR,   
	SAMPLEOUTOFBOUNDS, 
	TOOBIG,       
	TOOSMALL,
	BADALLOCATION,
} MovieStatus;

#pragma pack(push, 1)
typedef struct
{
	unsigned long	Magic;
	unsigned long	Version;
	unsigned long	UID;

	unsigned long	VIs;
	unsigned long	Rerecords;
	unsigned char   VIPerSecond;
	unsigned char   Controllers;
	unsigned short  _Reserved;
	unsigned long	Samples;

	unsigned short	StartFlags;
	unsigned short  _Reserved2;
	unsigned long	ControllerFlags;
	unsigned long	_ReservedFlags[8];

	char			OldAuthorInfo[48];
	char			OldDescription[80];
	char			RomName[32];
	unsigned long	RomCRC;
	unsigned short	RomCountry;
	char			ReservedBytes[56];
	char			VideoPluginName[64];
	char			SoundPluginName[64];
	char			InputPluginName[64];
	char			RSPPluginName[64];
	char			AuthorInfo[222];
	char			Description[256];
} SMovieHeader;


/// <summary>
/// Parses M64 and fills parameters with data. 
/// 
/// IMPORTANT: The host must provide and allocate MovieHeader and Inputs (pointer).
/// </summary>
/// <param name="FilePtr">Pointer to valid FILE structure</param>
/// <param name="HeaderPtr">Pointer to initialize SMovieHeader structure</param>
/// <param name="InputsPtr">Pointer to appropriately size-allocated SAMPLETYPE pointer array</param>
/// <returns>MovieStatus structure with information about operation status</returns>
extern MovieStatus Parse(FILE* FilePtr, SMovieHeader* HeaderPtr, SAMPLETYPE* InputsPtr);
/// <summary>
/// Parses M64 and fills parameters with data. 
/// 
/// IMPORTANT: The host must provide and allocate MovieHeader.
/// </summary>
/// <param name="FilePtr">Pointer to valid FILE structure</param>
/// <param name="HeaderPtr">Pointer to initialize SMovieHeader structure</param>
/// <param name="InputsPtr">Pointer to SAMPLETYPE pointer array which must be allocated with malloc</param>
/// <returns>MovieStatus structure with information about operation status</returns>
extern MovieStatus Parse2(FILE* FilePtr, SMovieHeader* HeaderPtr, SAMPLETYPE* InputsPtr);

#pragma pack(pop)