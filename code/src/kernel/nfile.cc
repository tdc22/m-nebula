#define N_IMPLEMENTS nFile
#define N_KERNEL
//------------------------------------------------------------------------------
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "kernel/nfile.h"
#include "kernel/nfileserver2.h"

//------------------------------------------------------------------------------
/**

	history:
    - 30-Jan-2002   peter   created
    - 11-Feb-2002   floh    Linux stuff
*/
nFile::nFile(nFileServer2* server) : fs(server), index(-1)
{	
	fs->AddFile(this);
#ifdef __WIN32__
    this->handle = 0;
#else
    this->fp = 0;
#endif
}

//------------------------------------------------------------------------------
/**

	history:
    - 30-Jan-2002   peter    created
*/
nFile::~nFile()
{
	if(this->IsOpen())
	{
		n_printf("Warning: File destroyed before closing\n");
		this->Close();
	}

	this->fs->FreeFile(this->index);
}

//------------------------------------------------------------------------------
/**
    checks existence of the specified file

	@param path - the name of the file to check    
    @return				success

	history:    
    + 27-Jan-2003   created, Ilya    Clockwise
*/
bool nFile::Exists(const char* path) {
    if (this->Open(path, "r")) 
	{
        this->Close();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    opens the specified file

	@param dirName		the name of the file to open
    @param accessMode   the access mode ("(r|w|a)[+]")
    @return				success

	history:
    - 30-Jan-2002   peter   created
    - 11-Feb-2002   floh    Linux stuff
*/
bool
nFile::Open(const char* fileName, const char* accessMode)
{
	n_assert(!this->IsOpen());

    n_assert(fileName);
    n_assert(accessMode);
    n_assert(strlen(fileName)>0);

	stl_string path;
	this->fs->MakeAbsoluteMangled(fileName, path);

#ifdef __WIN32__
	DWORD access = 0;
	DWORD disposition = 0;
    DWORD shareMode = 0;
        
	switch (accessMode[0]) {
	    case 'r':
	    case 'R':
            disposition = OPEN_EXISTING;
            access = GENERIC_READ;
            shareMode = FILE_SHARE_READ;
		    break;
	    case 'w':
	    case 'W':
	        disposition = CREATE_ALWAYS;
	        access = GENERIC_WRITE;
            shareMode = 0;

            break;
	    case 'a':
	    case 'A':
			disposition = OPEN_ALWAYS;
			access = GENERIC_WRITE;
            shareMode = 0;
            break;
	};

	this->handle = CreateFileA(path.c_str(),              // filename
                              access,           // access mode
                              shareMode,        // share mode
                              0,                // security flags
			                  disposition,      // what to do if file doesn't exist
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,  // flags'n'attributes
                              0);               // template file
	if (this->handle == INVALID_HANDLE_VALUE)
	{
		this->handle = 0;
		return false;
	}
#else
    this->fp = fopen(path.c_str(), accessMode);
    if (!this->fp)
    {
        return false;
    }
#endif

	return true;
}

//------------------------------------------------------------------------------
/**
	closes the file

	history:
    - 30-Jan-2002   peter   created
    - 11-Feb-2002   floh    Linux stuff
*/
void
nFile::Close()
{
	n_assert(this->IsOpen());

#ifdef __WIN32__
	if (this->handle)
    {
		CloseHandle(this->handle);
    	this->handle = 0;
    }
#else
    if (this->fp)
    {
        fclose(this->fp);
        this->fp = 0;
    }
#endif
}

//------------------------------------------------------------------------------
/**
    writes a number of bytes to the file

	@param buffer		buffer with data
	@param numBytes		number of bytes to write
    @return				number of bytes written

	history:
    - 30-Jan-2002   peter    created
    - 11-Feb-2002   floh    Linux stuff
*/
int
nFile::Write(const void* buffer, int numBytes)
{
	n_assert(this->IsOpen());
    
    // statistics
    this->fs->AddBytesWritten(numBytes);

#ifdef __WIN32__
	DWORD written;
	WriteFile(this->handle, buffer, numBytes, &written, NULL);
	return written;
#else
    return fwrite(buffer, 1, numBytes, this->fp);
#endif
}

//------------------------------------------------------------------------------
/**
    reads a number of bytes from the file

	@param buffer		buffer for data
	@param numBytes		number of bytes to read
	@return				number of bytes read

	history:
    - 30-Jan-2002   peter    created
*/
int
nFile::Read(void* buffer, int numBytes) const
{
	n_assert(this->IsOpen());

    // statistics
    this->fs->AddBytesRead(numBytes);

#ifdef __WIN32__
	DWORD read;
	ReadFile(this->handle, buffer, numBytes, &read, NULL);
	return read;
#else
    return fread(buffer, 1, numBytes, this->fp);
#endif
}


//------------------------------------------------------------------------------
/**
	gets current position of file pointer
  
	@return          position of pointer

	history:
    - 30-Jan-2002   peter    created
*/
int 
nFile::Tell() const
{
	n_assert(this->IsOpen());
#ifdef __WIN32__
	return SetFilePointer(this->handle, 0, NULL, FILE_CURRENT);
#else
    return ftell(this->fp);
#endif
}

//------------------------------------------------------------------------------
/**
	returns size of the file

	@return          size of the file

	history:
	- 1-May-2007   Ilya    created (day of proletariat they made me do it)
*/
int
nFile::Size() const 
{
	int cur = this->Tell();
	this->Seek(0, nFile::END);
	int sz = this->Tell();
	this->Seek(cur, nFile::START);
	return sz;
}

//------------------------------------------------------------------------------
/**
	sets the file pointer to given absolute or relative position

    @param byteOffset		the offset
	@param origin			position from which to count
	@return					success

	history:
    - 30-Jan-2002   peter    created
*/
bool 
nFile::Seek(int byteOffset, nSeekType origin) const
{
	n_assert(this->IsOpen());

    this->fs->AddSeek();

#ifdef __WIN32__
	DWORD method = FILE_BEGIN;
	switch (origin)
    {
	    case CURRENT:
		    method = FILE_CURRENT;
		    break;
	    case START:
		    method = FILE_BEGIN;
		    break;
	    case END:
		    method = FILE_END;
		    break;
	}

	DWORD ret = SetFilePointer(this->handle, (LONG)byteOffset, NULL, method);
	return (ret != 0xffffffff);
#else
    int whence = SEEK_SET;
    switch (origin)
    {
        case CURRENT:
            whence = SEEK_CUR;
            break;
        case START:
            whence = SEEK_SET;
            break;
        case END:
            whence = SEEK_END;
            break;
    }
    return (0 == fseek(this->fp, byteOffset, whence)) ? true : false;
#endif
}


//------------------------------------------------------------------------------
/**
    writes a string to the file
	  
	@param buffer		the string to write
	@return				success

	history:
    - 30-Jan-2002   peter    created
*/
bool
nFile::PutS(const char* buffer)
{
    n_assert(this->IsOpen());

	if (!buffer || !strlen(buffer))
		return true;

#ifdef __WIN32__
	int len = strlen(buffer);
	int written = this->Write(buffer, len);

	if (written != len)
		return false;
	else
		return true;
#else
    return (fputs(buffer, this->fp) >= 0);
#endif
}

//------------------------------------------------------------------------------
/**
    reads a string from the file up to and including the first newline character
	or up to the end of the buffer

	@param buffer			buffer for string
	@param numChars			maximum number of chars to read
	@return					success (false if eof is reached)

	history:
    - 30-Jan-2002   peter    created
*/
bool
nFile::GetS(char* buffer, int numChars)
{
    n_assert(this->IsOpen());
    n_assert(buffer);
    n_assert(numChars > 0);

#ifdef __WIN32__
    // store start filepointer position
    int seekPos = this->Tell();

    // read 64 bytes at once, and scan for newlines
    const int chunkSize = 64;
    int readSize  = chunkSize;
    char* readPos = buffer;

    bool retval = false;
    DWORD bytesRead = 0;
    int curIndex = 0;
    for (curIndex = 0; curIndex < (numChars - 1); curIndex++)
    {
        // read next chunk of data?
        if (0 == (curIndex % chunkSize))
        {
            readSize = chunkSize;
            if ((curIndex + readSize) >= numChars)
            {
                readSize = numChars - curIndex;
            }
            bytesRead = this->Read(readPos, readSize);
            readPos += readSize;
        }

        // end of line reached?
        if (0 == bytesRead)
        {
            retval = false;
            break;
        }

        // newline?
        if (buffer[curIndex] == '\n')
        {
            retval = true;
            this->Seek(seekPos + curIndex + 1, START);
            break;
        }
    }

    // terminate buffer
    buffer[curIndex] = 0;
    return retval;
#else
    return (fgets(buffer, numChars, this->fp) != 0);
#endif
}



//------------------------------------------------------------------------------
/**
	writes an unicode string to the file

	@param buffer		the string to write
	@return				success

	history:
	- 09-Aug-2008   Ilya    created
*/
bool
nFile::PutU(const wchar* buffer)
{
	n_assert(this->IsOpen());

#ifdef __WIN32__
	int len = wcslen(buffer)*sizeof(wchar);
	int written = this->Write(buffer, len);

	if (written != len)
		return false;
	else
		return true;
#else
	return (fputws(buffer, this->fp) >= 0);
#endif
}

//------------------------------------------------------------------------------
/**
	reads an unicode string from the file up to and including the first newline character
	or up to the end of the buffer

	@param buffer			buffer for string
	@param numChars			maximum number of chars to read
	@return					success (false if eof is reached)

	history:
	- 09-Aug-2008   Ilya    created
*/
bool
nFile::GetU(wchar* buffer, int numChars)
{
	n_assert(this->IsOpen());
	n_assert(buffer);
	n_assert(numChars > 0);

#ifdef __WIN32__
	// store start filepointer position
	int seekPos = this->Tell();

	// read 64 bytes at once, and scan for newlines
	const int chunkSize = 64;
	int readSize  = chunkSize;
	wchar* readPos = buffer;

	bool retval = false;
	DWORD bytesRead = 0;
	int curIndex = 0;
	for (curIndex = 0; curIndex < (numChars - 1); curIndex++)
	{
		// read next chunk of data?
		if (0 == (curIndex % chunkSize))
		{
			readSize = chunkSize;
			if ((curIndex + readSize) >= numChars)
			{
				readSize = numChars - curIndex;
			}
			bytesRead = this->Read(readPos, readSize*sizeof(wchar));
			readPos += readSize;
		}

		// end of line reached?
		if (0 == bytesRead)
		{
			retval = false;
			break;
		}

		// newline?
		if (buffer[curIndex] == '\n')
		{
			retval = true;
			this->Seek(seekPos + curIndex + 1, START);
			break;
		}
	}

	// terminate buffer
	buffer[curIndex] = 0;
	return retval;
#else
	return (fgetws(buffer, numChars, this->fp) != 0);
#endif
}
