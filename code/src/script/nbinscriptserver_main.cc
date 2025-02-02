#define N_IMPLEMENTS nBinScriptServer
//------------------------------------------------------------------------------
//  nbinscriptserver_main.cc
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "script/nbinscriptserver.h"
#include "kernel/nfileserver2.h"
#include "kernel/ncmdprotonative.h"

#if defined(GetObject)
// For that fuzzy windows feeling
// VC6 chokes on this one on for some reason - thinks it is GetObjectA
// nBinScriptServer::GetObject()
#undef GetObject
#endif


nNebulaClass(nBinScriptServer, "nscriptserver");

//------------------------------------------------------------------------------
/**
*/
nBinScriptServer::nBinScriptServer() :
    refFileServer(kernelServer, this)
{
    this->refFileServer = "/sys/servers/file2";
	this->extensions.push_back(stl_string("nb"));
}

//------------------------------------------------------------------------------
/**
*/
nBinScriptServer::~nBinScriptServer()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Begin writing a persistent object.
*/
nFile*
nBinScriptServer::BeginWrite(const char* filename, nRoot* obj)
{
    n_assert(filename);
    n_assert(obj);

    nFile* file = this->refFileServer->NewFileObject();
    n_assert(file);
    if (file->Open(filename, "wb"))
    {
        // write magic number
        this->PutInt(file, 'NOB0');

        // write parser class and wrapper object class
        char buf[N_MAXPATH];
        sprintf(buf, "$parser:nbinscriptserver$ $class:%s$", obj->GetClass()->GetName());
        this->PutString(file, buf);
        return file;
    }
    else
    {
        n_printf("nBinScriptServer::WriteBegin(): failed to open file '%s' for writing!\n", filename);
        delete file;
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
    Finish writing a persistent object.
*/
bool
nBinScriptServer::EndWrite(nFile* file)
{
    n_assert(file);
    file->Close();
    delete file;
    return true;
}

//------------------------------------------------------------------------------
/**
    Starts a new object in a persistent object file.
*/
bool
nBinScriptServer::WriteBeginNewObject(nFile* file, nRoot* obj, nRoot* owner)
{
    n_assert(file);
    n_assert(obj);
    n_assert(owner);

    const char* objName  = obj->GetName();
    const char* objClass = obj->GetClass()->GetName();

    this->PutInt(file, '_new');
    this->PutString(file, objClass);
    this->PutString(file, objName);

    return true;
}

//------------------------------------------------------------------------------
/**
    Write start of new object with constructor command.
    
    @warning Any nCmd passed in to the binary script server must have an 
    nCmdProto of the nCmdProtoNative type, if that is not the case bad things
    will happen.
*/
bool
nBinScriptServer::WriteBeginNewObjectCmd(nFile* file, nRoot* obj, nRoot* owner, nCmd* cmd)
{
    n_assert(file);
    n_assert(obj);
    n_assert(owner);
    n_assert(cmd);

    this->WriteCmd(file, cmd);

    return true;
}

//------------------------------------------------------------------------------
/**
    Write start of persistent object without constructor, only the
    select statement.
*/
bool
nBinScriptServer::WriteBeginSelObject(nFile* file, nRoot* obj, nRoot* owner)
{
    n_assert(file);
    n_assert(obj);
    n_assert(owner);

    this->WriteSelect(file, obj, owner, this->GetSelectMethod());

    return true;
}

//------------------------------------------------------------------------------
/**
    Finish a persistent object, this puts a _sel command which restores
    the "owner" object as current working object.
*/
bool
nBinScriptServer::WriteEndObject(nFile* file, nRoot* obj, nRoot* owner)
{
    n_assert(file);
    n_assert(obj);
    n_assert(owner);

    this->WriteSelect(file, owner, obj, SELCOMMAND);

    return true;
}

//------------------------------------------------------------------------------
/**
    Write a select statement which changes the cwd from 'obj1' to 'obj0'.
*/
void
nBinScriptServer::WriteSelect(nFile* file, nRoot* obj0, nRoot* obj1, nScriptServer::SelectMethod selMethod)
{
    n_assert(file);
    n_assert(obj0);
    n_assert(obj1);

    switch (selMethod)
    {
        case SELCOMMAND:
            // get relative path from obj1 to obj0 and write select statement
			{
				stl_string relpath;            
				this->PutInt(file, '_sel');
				this->PutString(file, obj1->GetRelPath(obj0, relpath));
			}
            break;

        case NOSELCOMMAND:
            break;
    }
}

//------------------------------------------------------------------------------
/**
    Returns the length in bytes which the nCmd's arguments would take
    in the persistent object file.
*/
int
nBinScriptServer::GetArgLength(nCmd* cmd)
{
    int len = 0;

    cmd->Rewind();
    int numArgs = cmd->GetNumInArgs();
    int i;
    for (i = 0; i < numArgs; i++)
    {
        nArg* arg = cmd->In();

        switch (arg->GetType())
        {
            case nArg::ARGTYPE_INT:
                len += sizeof(int);
                break;

            case nArg::ARGTYPE_FLOAT:
                len += sizeof(float);
                break;

            case nArg::ARGTYPE_STRING:
                len += strlen(arg->GetS()) + sizeof(ushort);
                break;

			case nArg::ARGTYPE_USTRING:
				len += wcslen(arg->GetU())*sizeof(wchar) + sizeof(ushort);
				break;

            case nArg::ARGTYPE_CODE:
                len += strlen(arg->GetC()) + sizeof(ushort);
                break;

            case nArg::ARGTYPE_BOOL:
                len += sizeof(char);
                break;

            case nArg::ARGTYPE_OBJECT:
                {                    
                    nRoot* obj = (nRoot*) arg->GetO();                    
                    len += obj->GetFullName().size() + sizeof(ushort);
                }
                break;

            case nArg::ARGTYPE_VOID:
                break;

            case nArg::ARGTYPE_LIST:
                n_error("ARGTYPE_LIST not implemented yet");
                break;

        }
    }
    n_assert(len < (1<<15));
    return len;
}

//------------------------------------------------------------------------------
/**
    Write a nCmd object to the file.
    
    @warning Any nCmd passed in to the binary script server must have an 
    nCmdProto of the nCmdProtoNative type, if that is not the case bad things
    will happen.
*/
bool
nBinScriptServer::WriteCmd(nFile* file, nCmd* cmd)
{
    n_assert(file);
    n_assert(cmd);

    // write cmd fourcc
    this->PutInt(file, ((nCmdProtoNative*)cmd->GetProto())->GetID());

    // get summed byte length of arguments, and write length (needed to
    // skip cmd if the target object doesn't know about the command)
    this->PutShort(file, (short)this->GetArgLength(cmd));

    // write command args
    cmd->Rewind();
    int numArgs = cmd->GetNumInArgs();
    int i;
    for (i = 0; i < numArgs; i++)
    {
        nArg* arg = cmd->In();

        switch (arg->GetType())
        {
            case nArg::ARGTYPE_INT:
                this->PutInt(file, arg->GetI());
                break;

            case nArg::ARGTYPE_FLOAT:
                this->PutFloat(file, arg->GetF());
                break;

            case nArg::ARGTYPE_STRING:
                this->PutString(file, arg->GetS());
                break;

			case nArg::ARGTYPE_USTRING:
				this->PutUString(file, arg->GetU());
				break;

            case nArg::ARGTYPE_CODE:
                this->PutCode(file, arg->GetC());
                break;

            case nArg::ARGTYPE_BOOL:
                this->PutBool(file, arg->GetB());
                break;

            case nArg::ARGTYPE_OBJECT:
                this->PutObject(file, (nRoot*) arg->GetO());
                break;

            case nArg::ARGTYPE_VOID:
                break;

            case nArg::ARGTYPE_LIST:
                n_error("ARGTYPE_LIST not implemented yet");
                break;

        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Write an 32 bit int to the file.
*/
void
nBinScriptServer::PutInt(nFile* file, int val)
{
    n_assert(file);
    file->Write(&val, sizeof(int));
}

//------------------------------------------------------------------------------
/**
    Write a 16 bit int to the file
*/
void
nBinScriptServer::PutShort(nFile* file, short val)
{
    n_assert(file);
    file->Write(&val, sizeof(short));
}

//------------------------------------------------------------------------------
/**
    Write an float to the file.
*/
void
nBinScriptServer::PutFloat(nFile* file, float val)
{
    n_assert(file);
    file->Write(&val, sizeof(float));
}

//------------------------------------------------------------------------------
/**
    Write a string to the file.
*/
void
nBinScriptServer::PutString(nFile* file, const char* str)
{
    n_assert(file);
    n_assert(str);

    // write string length
    ushort strLen = (ushort)strlen(str);
    file->Write(&strLen, sizeof(ushort));

    // write string
    file->Write(str, strLen);
}

//------------------------------------------------------------------------------
/**
	Write a unicode string to the file.
*/
void
nBinScriptServer::PutUString(nFile* file, const wchar* str)
{
	n_assert(file);
	n_assert(str);

	// write string length
	ushort strLen = (ushort)wcslen(str);
	size_t strSz = strLen*sizeof(wchar);
	file->Write(&strLen, sizeof(ushort));
	
	// write string
	file->Write(str, strSz);
}

//------------------------------------------------------------------------------
/**
    Write a bool to the file.
*/
void
nBinScriptServer::PutBool(nFile* file, bool b)
{
    n_assert(file);

    char c = b ? 1 : 0;
    file->Write(&c, sizeof(char));
}

//------------------------------------------------------------------------------
/**
    Write script code to the file.
*/
void
nBinScriptServer::PutCode(nFile* file, const char* str)
{
    n_assert(file);
    this->PutString(file, str);
}

//------------------------------------------------------------------------------
/**
    Write object handle to the file.
*/
void
nBinScriptServer::PutObject(nFile* file, nRoot* obj)
{
    n_assert(file);
    if (obj)
    {        
        this->PutString(file, obj->GetFullName().c_str());
    }
    else
    {
        this->PutString(file, "null");
    }
}

//------------------------------------------------------------------------------
/**
    Read a 32 bit int from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetInt(nFile* file, int& val)
{
    n_assert(file);
    int bytesRead = file->Read(&val, sizeof(int));
    return sizeof(int) == bytesRead;
}

//------------------------------------------------------------------------------
/**
    Read a 16 bit short from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetShort(nFile* file, short& val)
{
    n_assert(file);
    int bytesRead = file->Read(&val, sizeof(short));
    return sizeof(short) == bytesRead;
}

//------------------------------------------------------------------------------
/**
    Read a float from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetFloat(nFile* file, float& val)
{
    n_assert(file);
    int bytesRead = file->Read(&val, sizeof(float));
    return sizeof(float) == bytesRead;
}

//------------------------------------------------------------------------------
/**
    Read a string from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetString(nFile* file, stl_string& val)
{
    n_assert(file);
    int bytesRead;

    // read length of string
    ushort strLen;
    bytesRead = file->Read(&strLen, sizeof(ushort));
    if (sizeof(ushort) != bytesRead)
    {
        return false;
    }

	val.resize(strLen + 1);
    // read string    
    bytesRead = file->Read(&val[0], strLen);
	val[strLen] = 0;
    if (bytesRead != strLen)
	{
        val.clear();
    }	
    return (bytesRead == strLen);
}

//------------------------------------------------------------------------------
/**
	Read a unicode string from the file.

	@param  file    [in]  nFile object to read from
	@param  val     [out] read value
	@return         false if EOF reached
*/
bool
nBinScriptServer::GetUString(nFile* file, stl_wstring& val)
{
	n_assert(file);
	int bytesRead;

	// read length of string
	ushort strLen;
	bytesRead = file->Read(&strLen, sizeof(ushort));
	if (sizeof(ushort) != bytesRead)
	{
		return false;
	}

	val.resize(strLen + 1);
	// read string    
	bytesRead = file->Read(&val[0], strLen*sizeof(wchar));
	val[strLen] = 0;
	if (bytesRead != strLen)
	{
		val.clear();
	}	
	return (bytesRead == strLen);
}

//------------------------------------------------------------------------------
/**
    Read a bool from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetBool(nFile* file, bool& val)
{
    n_assert(file);
    char c;
    int bytesRead = file->Read(&c, sizeof(char));
    val = (0 == c) ? false : true;
    return (sizeof(char) == bytesRead);
}

//------------------------------------------------------------------------------
/**
    Read a code sequence from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetCode(nFile* file, stl_string& val)
{
    n_assert(file);
    return this->GetString(file, val);
}

//------------------------------------------------------------------------------
/**
    Read an object handle from the file.

    @param  file    [in]  nFile object to read from
    @param  val     [out] read value
    @return         false if EOF reached
*/
bool
nBinScriptServer::GetObject(nFile* file, nRoot*& val)
{
    n_assert(file);

    // get object string handle from file
    stl_string objHandle;
    if (this->GetString(file, objHandle))
    {
        if (strcmp(objHandle.c_str(), "null") == 0)
        {
            // special case null object
            val = 0;
        }
        else
        {
            // lookup object
            val = kernelServer->Lookup(objHandle.c_str());
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**    
    Verify that this is a NOB0 file, and skip the header. Return false
    if EOF reached for some reason.
*/
bool
nBinScriptServer::GetHeader(nFile* file)
{
    n_assert(file);

    // read and verify the magic number
    int magic;
    if (this->GetInt(file, magic))
    {
        if ('NOB0' != magic)
        {
            n_printf("nBinScriptServer::GetHeader(): not a binary Nebula persistent object file!\n");
            return false;
        }

        // skip the header string
        stl_string dummyStr;
        return this->GetString(file, dummyStr);
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**    
    Read input args from file and write to nCmd object
    
    @param  file    file to read from
    @param  cmd     nCmd object to initialize
    @return         false if eof reached
*/
bool
nBinScriptServer::GetInArgs(nFile* file, nCmd* cmd)
{
    n_assert(file);
    n_assert(cmd);

    int i;
    int iArg;
    float fArg;
    stl_string sArg;
	stl_wstring uArg;
    stl_string cArg;
    bool bArg;
    nRoot* oArg;

    cmd->Rewind();
    int numArgs = cmd->GetNumInArgs();
    for (i = 0; i < numArgs; i++)
    {
        bool notEof = true;
        nArg* arg = cmd->In();

        switch(arg->GetType())
        {
        
            case nArg::ARGTYPE_INT:
                notEof = this->GetInt(file, iArg);
                arg->SetI(iArg);
                break;

            case nArg::ARGTYPE_FLOAT:
                notEof = this->GetFloat(file, fArg);
                arg->SetF(fArg);
                break;

            case nArg::ARGTYPE_STRING:
                notEof = this->GetString(file, sArg);
                arg->SetS(sArg.c_str());
                break;

			case nArg::ARGTYPE_USTRING:
				notEof = this->GetUString(file, uArg);
				arg->SetU(uArg.c_str());
				break;

            case nArg::ARGTYPE_CODE:
                notEof = this->GetCode(file, cArg);
                arg->SetC(cArg.c_str());
                break;

            case nArg::ARGTYPE_BOOL:
                notEof = this->GetBool(file, bArg);
                arg->SetB(bArg);
                break;

            case nArg::ARGTYPE_OBJECT:
                notEof = this->GetObject(file, oArg);
                arg->SetO(oArg);
                break;

            case nArg::ARGTYPE_VOID:
                break;

            case nArg::ARGTYPE_LIST:
                n_error("ARGTYPE_LIST not implemented yet");
                break;

        }

        if (!notEof)
        {
            // eof reached
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**    
    Read and execute a cmd block. This may create new objects and change
    Nebula's currently selected object.

    @param  file    file to read from
    @return         false if EOF reached
*/
bool
nBinScriptServer::ReadBlock(nFile* file)
{
    bool notEof;

    // read next fourcc, return false if EOF reached
    int fourcc;
    notEof = this->GetInt(file, fourcc);
    if (!notEof)
    {
        // eof reached
        return false;
    }

    // handle special commands '_new' and '_sel'
    if ('_new' == fourcc)
    {

        // read class and object name (necessary to COPY the strings!)
        stl_string objClass, objName;
        notEof = this->GetString(file, objClass);
        n_assert(notEof);
        notEof = this->GetString(file, objName);
        n_assert(notEof);

        // create object and select it
        nRoot* obj = kernelServer->New(objClass.c_str(), objName.c_str());
        if (obj)
        {
            kernelServer->SetCwd(obj);
        }
        else
        {
            n_error("nBinScriptServer::ReadBlock(): '_new %s %s' failed!\n", objClass.c_str(), objName.c_str());
        }
    }
    else if ('_sel' == fourcc)
    {
        // read relative path
        stl_string relPath;
        notEof = this->GetString(file, relPath);
        n_assert(notEof);

        nRoot* obj = kernelServer->Lookup(relPath.c_str());
        if (obj)
        {
            kernelServer->SetCwd(obj);
        }
        else
        {
            n_error("nBinScriptServer::ReadBlock(): '_sel %s' failed!\n", relPath.c_str());
        }
    }
    else
    {
        // this is a normal cmd
        nRoot* obj = kernelServer->GetCwd();
        n_assert(obj);
        nClass* objClass = obj->GetClass();
        nCmdProto* cmdProto = objClass->FindCmdById(fourcc);
        if (cmdProto)
        {
            nCmd* cmd = cmdProto->NewCmd();
            n_assert(cmd);

            // skip in args length
            short dummy;
            notEof = this->GetShort(file, dummy);
            n_assert(notEof);

            // read input args into cmd object
            this->GetInArgs(file, cmd);

            // invoke cmd on current object
            bool success = obj->Dispatch(cmd);
            if (!success)
            {                
                n_printf("nBinScriptServer::ReadBlock(): obj '%s' doesn't accept cmd '%s'\n", 
                    obj->GetFullName(), cmdProto->GetName());
            }
            cmdProto->RelCmd(cmd);
        }
        else
        {
            // the object doesn't know the command, skip it
            short argLen;
            notEof = this->GetShort(file, argLen);
            n_assert(notEof);
            notEof = file->Seek(argLen, nFile::CURRENT);
            n_assert(notEof);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
/**    
    Evaluate a NOB0 file.
*/
bool
nBinScriptServer::RunScript(const char* filename, const char*& result)
{
    bool retval = false;
    result = 0;

    // create and open file object
    nFile* file = this->refFileServer->NewFileObject();
    n_assert(file);
    if (file->Open(filename, "rb"))
    {
        // verify header
        if (!this->GetHeader(file))
        {
            n_printf("nBinScriptServer::RunScript(): '%s' not a NOB0 file!\n", filename);
            return false;
        }

        // push cwd
        kernelServer->PushCwd(kernelServer->GetCwd());

        // read and execute blocks
        while (this->ReadBlock(file));

        // pop cwd
        kernelServer->PopCwd();

        file->Close();
        retval = true;
    }
    else
    {
        n_printf("nBinScriptServer::RunScript(): could not open file '%s'\n", filename);
    }
    delete file;
    return retval;
}

//------------------------------------------------------------------------------

