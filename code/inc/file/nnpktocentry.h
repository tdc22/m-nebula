#ifndef N_NPKTOCENTRY_H
#define N_NPKTOCENTRY_H
//------------------------------------------------------------------------------
/**
    @class nNpkTocEntry
    @ingroup NpkFileServer
    
    @brief A table of content entry in a nNpkToc object. Toc entries are organized
    in a tree of nodes. A node can be a dir node, or a file node. File nodes
    never have children, dir nodes can have children but don't have to.

    (C) 2002 RadonLabs GmbH
*/
#ifndef N_HASHNODE_H
#include "util/nhashnode.h"
#endif

#ifndef N_HASHLIST_H
#include "util/nhashlist.h"
#endif

//------------------------------------------------------------------------------
class nNpkFileWrapper;
class nNpkTocEntry : public nHashNode
{
public:
    /// entry types
    enum Type
    {
        DIR,
        FILE
    };

    /// constructor for directory entry
    nNpkTocEntry(const char* rootPath, nNpkTocEntry* parentEntry, const char* entryName);
    /// constructor for file entry
    nNpkTocEntry(const char* rootPath, nNpkTocEntry* parentEntry, const char* entryName, int fileOffset, int fileLength);
    /// destructor
    ~nNpkTocEntry();
    /// get parent entry
    nNpkTocEntry* GetParent() const;
    /// get entry type
    Type GetType() const;
    /// get file offset
    int GetFileOffset() const;
    /// get file length
    int GetFileLength() const;
    /// get root path
    const char* GetRootPath() const;
    /// add a sub dir entry
    nNpkTocEntry* AddDirEntry(const char* entryName);
    /// add a file entry
    nNpkTocEntry* AddFileEntry(const char* entryName, int fileOffset, int fileLength);
    /// find a sub entry by name (dirs only)
    nNpkTocEntry* FindEntry(const char* name);
    /// get the first entry
    nNpkTocEntry* GetFirstEntry();
    /// get the next entry
    nNpkTocEntry* GetNextEntry(nNpkTocEntry* curEntry);
    /// get full entry path name
    const char* GetFullName(char* buf, int bufSize);
    /// set the file wrapper object which this tocEntry belongs to
    void SetFileWrapper(nNpkFileWrapper* wrapper);
    /// get the file wrapper object which this tocEntry belongs to
    nNpkFileWrapper* GetFileWrapper() const;
    /// set the duplicate flag
    void SetDuplicate(bool b);
    /// get the duplicate flag
    bool GetDuplicate() const;

private:
    nNpkFileWrapper* fileWrapper;
    const char* rootPath;   // root path string (not owned!)
    nNpkTocEntry* parent;
    Type type;
    int offset;
    int length;
    nHashList* entryList;   // optional hash list
    bool duplicate;         // this is a duplicate of a file in an more recent npk file
};

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry::nNpkTocEntry(const char* rPath, nNpkTocEntry* parentEntry, const char* entryName) :
    nHashNode(entryName),
    fileWrapper(0),
    rootPath(rPath),
    parent(parentEntry),
    type(DIR),
    offset(0),
    length(0),
    entryList(0),
    duplicate(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry::nNpkTocEntry(const char* rPath, nNpkTocEntry* parentEntry, const char* entryName, int fileOffset, int fileLength) :
    nHashNode(entryName),
    fileWrapper(0),
    rootPath(rPath),
    parent(parentEntry),
    type(FILE),
    offset(fileOffset),
    length(fileLength),
    entryList(0),
    duplicate(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry::~nNpkTocEntry()
{
    if (this->entryList)
    {
        nNpkTocEntry* curEntry;
        while ((curEntry = (nNpkTocEntry*) this->entryList->RemHead()))
        {
            n_delete curEntry;
        }
        n_delete this->entryList;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::GetParent() const
{
    return this->parent;
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry::Type
nNpkTocEntry::GetType() const
{
    return this->type;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nNpkTocEntry::GetFileOffset() const
{
    return this->offset;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nNpkTocEntry::GetFileLength() const
{
    return this->length;
}

//------------------------------------------------------------------------------
/**
    This is the path to the NPK file which contains this toc entry,
    NOT the actual path to the entry itself!
*/
inline
const char*
nNpkTocEntry::GetRootPath() const
{
    return this->rootPath;
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::AddDirEntry(const char* entryName)
{
    n_assert(DIR == this->type);

    // create entry list object on demand
    if (!this->entryList)
    {
        this->entryList = n_new nHashList(32);
    }

    // create and add entry
    nNpkTocEntry* newEntry = n_new nNpkTocEntry(this->GetRootPath(), this, entryName);
    this->entryList->AddTail(newEntry);
    return newEntry;
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::AddFileEntry(const char* entryName, int fileOffset, int fileLength)
{
    n_assert(DIR == this->type);

    // create entry list object on demand
    if (!this->entryList)
    {
        this->entryList = n_new nHashList(32);
    }

    // create and add entry
    nNpkTocEntry* newEntry = n_new nNpkTocEntry(this->GetRootPath(), this, entryName, fileOffset, fileLength);
    this->entryList->AddTail(newEntry);
    return newEntry;
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::FindEntry(const char* entryName)
{
    n_assert(DIR == this->type);
    if (this->entryList)
    {
        return (nNpkTocEntry*) this->entryList->Find(entryName);
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::GetFirstEntry()
{
    n_assert(DIR == this->type);
    if (this->entryList)
    {
        nNpkTocEntry* entry;

        // filter out dupes
        for (entry = (nNpkTocEntry*) this->entryList->GetHead();
             entry;
             entry = (nNpkTocEntry*) entry->GetSucc())
        {
            if (!entry->GetDuplicate())
            {
                break;
            }
        }
        return entry;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkTocEntry*
nNpkTocEntry::GetNextEntry(nNpkTocEntry* curEntry)
{
    n_assert(DIR == this->type);
    n_assert(curEntry);

    // filter out dupes
    nNpkTocEntry* entry;
    for (entry = (nNpkTocEntry*) curEntry->GetSucc();
         entry;
         entry = (nNpkTocEntry*) entry->GetSucc())
    {
        if (!entry->GetDuplicate())
        {
            break;
        }
    }
    return entry;
}

//------------------------------------------------------------------------------
/**
    Get the full entry path name.
*/
inline
const char*
nNpkTocEntry::GetFullName(char* buf, int bufSize)
{
    n_assert(bufSize >= N_MAXPATH);
    static const int maxDepth = 16;
    nNpkTocEntry* traceStack[maxDepth];

    // fill trace stack with path to parent...
    int depth = 0;
    nNpkTocEntry* curEntry = this;
    while (curEntry && (depth < maxDepth))
    {
        traceStack[depth++] = curEntry;
        curEntry = curEntry->GetParent();
    }

    // fill buf with absolute path name to this entry
    int curBufIndex = 0;
    
    if (this->rootPath)
    {
        strcpy(buf, this->rootPath);
        curBufIndex = strlen(this->rootPath);
        buf[curBufIndex++] = '/';
    }

    for (--depth; depth >= 0; --depth)
    {
        const char* curName = traceStack[depth]->GetName();
        int curNameLen = strlen(curName);

        n_assert((curBufIndex + curNameLen + 1) < bufSize);

        strcpy(&(buf[curBufIndex]), curName);
        curBufIndex += curNameLen;
        if (depth > 0)
        {
            buf[curBufIndex++] = '/';
        }
    }
    n_assert(curBufIndex < bufSize);
    buf[curBufIndex] = 0;

    return buf;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nNpkTocEntry::SetFileWrapper(nNpkFileWrapper* wrapper)
{
    this->fileWrapper = wrapper;
}

//------------------------------------------------------------------------------
/**
*/
inline
nNpkFileWrapper*
nNpkTocEntry::GetFileWrapper() const
{
    return this->fileWrapper;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nNpkTocEntry::SetDuplicate(bool b)
{
    this->duplicate = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
nNpkTocEntry::GetDuplicate() const
{
    return this->duplicate;
}

//------------------------------------------------------------------------------
#endif
