#define N_IMPLEMENTS nPersistServer
#define N_KERNEL
//------------------------------------------------------------------------------
//  npersistserver_main.cc
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "kernel/nkernelserver.h"
#include "kernel/nscriptserver.h"
#include "kernel/npersistserver.h"
#include "kernel/ncmdprotonative.h"

//nNebulaScriptClass(nPersistServer, "nroot");

//------------------------------------------------------------------------------
/**
*/
nPersistServer::nPersistServer() :
    refFileServer(kernelServer, this),
    refSaver(kernelServer, this),
    file(0),
    saveMode(SAVEMODE_FOLD),
    cloneTarget(0),
    origCwd(0),
    saveLevel(0)
{
    this->refFileServer = "/sys/servers/file2";
    this->refSaver      = "/sys/servers/script";
}

//------------------------------------------------------------------------------
/**
*/
nPersistServer::~nPersistServer()
{
    n_assert(0 == this->file);
}

//------------------------------------------------------------------------------
/**
*/
void 
nPersistServer::SetSaveLevel(int l)
{
    this->saveLevel = l;
}

//------------------------------------------------------------------------------
/**
*/
int 
nPersistServer::GetSaveLevel(void)
{
    return this->saveLevel;
}

//------------------------------------------------------------------------------
/**
    Set class of script server which should be used to save objects.
    Creates a local script server object if not exists.

    @param  saverClass      name of a nScriptServer derived class
    @return                 currently always true, breaks if object could not be created
*/
bool
nPersistServer::SetSaverClass(const char* saverClass)
{
    n_assert(saverClass);

    // first check if the default script server matches this class
    nScriptServer* defaultServer = (nScriptServer*) kernelServer->Lookup("/sys/servers/script");
    if (defaultServer && (strcmp(saverClass, defaultServer->GetClass()->GetName()) == 0))
    {
        this->refSaver.set(0);
        this->refSaver = defaultServer;
    }
    else
    {
        // otherwise check if a matching local script server already exists
        nScriptServer* localServer = (nScriptServer*) this->Find(saverClass);
        if (!localServer)
        {
            kernelServer->PushCwd(this);
            localServer = (nScriptServer*) kernelServer->New(saverClass, saverClass);
            kernelServer->PopCwd();
        }
        n_assert(localServer);
        this->refSaver.set(0);
        this->refSaver = localServer;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Get name of class which is currently used to save objects.
*/
const char*
nPersistServer::GetSaverClass()
{
    return this->refSaver->GetClass()->GetName();
}

//------------------------------------------------------------------------------
/**
    Get a loader script server.
*/
nScriptServer*
nPersistServer::GetLoader(const char* loaderClass)
{
    n_assert(loaderClass);
    nScriptServer* loaderServer = 0;

    // first check if the default script server matches this class
    loaderServer = (nScriptServer*) kernelServer->Lookup("/sys/servers/script");
    if (loaderServer && (strcmp(loaderClass, loaderServer->GetClass()->GetName()) == 0))
    {
        // the default server is the right one
        return loaderServer;
    }
    else
    {
        // otherwise check if a matching local script server already exists
        loaderServer = (nScriptServer*) this->Find(loaderClass);
        if (!loaderServer)
        {
            // if not, create
            kernelServer->PushCwd(this);
            loaderServer = (nScriptServer*) kernelServer->New(loaderClass, loaderClass);
            kernelServer->PopCwd();
        }
        n_assert(loaderServer);
        return loaderServer;
    }   
}

//------------------------------------------------------------------------------
/**
    04-Nov-98   floh    created
    05-Nov-98   floh    autonome Objects gekillt
    18-Dec-98   floh    + SAVEMODE_CLONE
*/
bool 
nPersistServer::BeginObject(nRoot *o, const char *name)
{
    bool retval = false;
    switch (this->saveMode) 
    {
        case SAVEMODE_FOLD:   
            retval = this->BeginFoldedObject(o, NULL, name, false);
            break;
        case SAVEMODE_CLONE:
            retval = this->BeginCloneObject(o, name);
            break;
    }
    return retval;
}

//------------------------------------------------------------------------------
/**
    Start writing object, with constructor cmd instead of a
    generic "new [class] [name]". If cmd pointer is NULL,
    no constructor statement will be written, just the select
    statement.

    29-Feb-00   floh    created
*/
bool 
nPersistServer::BeginObjectWithCmd(nRoot *o, nCmd *cmd, const char *name)
{
    bool retval = false;
    bool sel_only = (cmd==NULL) ? true : false;  
    switch (this->saveMode) 
    {
        case SAVEMODE_FOLD:   
            retval = this->BeginFoldedObject(o, cmd, name, sel_only);
            break;
        case SAVEMODE_CLONE:
            retval = this->BeginCloneObject(o, name);
            break;
    }
    if (cmd) 
    {
        cmd->GetProto()->RelCmd(cmd);
    }
    return retval;
}

//------------------------------------------------------------------------------
/**
    04-Nov-98   floh    created
    05-Nov-98   floh    autonome Objects gekillt
    18-Dec-98   floh    + SAVEMODE_CLONE
*/
bool 
nPersistServer::EndObject()
{
    bool retval = false;
    switch (this->saveMode) 
    {
        case SAVEMODE_FOLD:   
            retval = this->EndFoldedObject();
            break;
        case SAVEMODE_CLONE:
            retval = this->EndCloneObject();
            break;
    }
    return retval;
}

//------------------------------------------------------------------------------
/**
    04-Nov-98   floh    created
    19-Dec-98   floh    neues Arg, id
                        + jetzt doch etwas komplexer, sucht das
                          Command selbst raus, indem es die
                          Klassen des Objekts durchsucht
    08-Aug-99   floh    + benutzt nicht mehr nCmdProto::GetCmd(),
                          sondern nCmdProto::NewCmd()
*/
nCmd*
nPersistServer::GetCmd(nRoot *o, uint id)
{
    n_assert(o);
    nCmdProto *cp = o->GetClass()->FindCmdById(id);
    n_assert(cp);
    return cp->NewCmd();
}

//------------------------------------------------------------------------------
/**
    Ignore if level is less or equal the internal save level
    defined by 'SetSaveLevel'.
    
    16-Jun-00   floh    created
*/
bool 
nPersistServer::PutCmd(int level, nCmd *cmd)
{
    bool success = false;
    if (this->saveLevel <= level) 
    {
        n_assert(cmd);
        bool success;
        if (SAVEMODE_CLONE == this->saveMode) 
        {
            // in clone mode, send cmd directly to object
            n_assert(this->cloneTarget);
            success = this->cloneTarget->Dispatch(cmd);
        } 
        else 
        {
            success = this->refSaver->WriteCmd(this->file, cmd);
        }
    }
    cmd->GetProto()->RelCmd(cmd);
    return success;
}

//------------------------------------------------------------------------------
/**
    This is the same as a PutCmd(0,cmd).

    04-Nov-98   floh    created
    08-Aug-99   floh    nCmd wird nicht mehr per delete(), sondern
                        per Cmd-Proto freigegeben
    20-Jan-00   floh    rewritten to ref_ss
*/
bool 
nPersistServer::PutCmd(nCmd *cmd)
{
    return this->PutCmd(0, cmd);
}

//------------------------------------------------------------------------------
/**
    05-Nov-98   floh    created
*/
void 
nPersistServer::SetSaveMode(nPersistServer::nSaveMode sm)
{
    this->saveMode = sm;
}

//------------------------------------------------------------------------------
/**
    05-Nov-98   floh    created
*/
nPersistServer::nSaveMode 
nPersistServer::GetSaveMode()
{
    return this->saveMode;
}

//------------------------------------------------------------------------------
//  EOF
//------------------------------------------------------------------------------
