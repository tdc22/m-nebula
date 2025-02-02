#ifndef N_REMOTESERVER_H
#define N_REMOTESERVER_H
//------------------------------------------------------------------------------
/**
    The remote server opens a socket and listens for connecting
    clients. For each client, a remote session will be created,
    each with its own current working object.

    All strings coming from the remote client will be send to the
    script server for evaluation. All strings emitted by 
    the Nebula kernel will be sent back to the client.

    Please check out the nRemoteClient class, which is designed to
    talk to the nRemoteServer class.

    (C) 2002 RadonLabs GmbH
*/
#ifndef N_ROOT_H
#include "kernel/nroot.h"
#endif

#ifndef N_IPCSERVER_H
#include "kernel/nipcserver.h"
#endif

#ifndef N_AUTOREF_H
#include "kernel/nautoref.h"
#endif

#undef N_DEFINES
#define N_DEFINES nRemoteServer
#include "kernel/ndefdllclass.h"

//------------------------------------------------------------------------------
class nScriptServer;
class N_PUBLIC nRemoteServer : public nRoot
{
public:
    /// constructor
    nRemoteServer();
    /// destructor
    virtual ~nRemoteServer();
    /// open a named communication port
    bool Open(const char* portName);
    /// close the communication port
    void Close();
    /// return true if open
    bool IsOpen() const;
    /// send a string to all connected clients
    bool Broadcast(const char* str, bool backline_only = false);
	/// send a data to all connected clients
	bool BroadcastData(const char* buf, int sz, bool backline_only = false);
    /// process pending messages, call this method frequently
    bool Trigger();

    static nKernelServer* kernelServer;
    static nClass *local_cl;

private:
    class nClientContext : public nNode
    {
        friend class nRemoteServer;

        /// constructor
        nClientContext(int cid, nRemoteServer* owner);
        /// destructor
        virtual ~nClientContext();
        /// get client id
        int GetClientId() const;
        /// set cwd object
        void SetCwd(nRoot* obj);
        /// get cwd object
        nRoot* GetCwd() const;

        int clientId;
        nRef<nRoot> refCwd;
    };

    /// get client context for clientid
    nClientContext* GetClientContext(int clientId);
    /// get cwd for a client id, create new client context if not exists yet
    nRoot* GetClientCwd(int clientId);
    /// set cwd in client context
    void SetClientCwd(int clientId, nRoot* cwd);


    nAutoRef<nScriptServer> refScriptServer;
    bool isOpen;
    nIpcServer* ipcServer;
    nList clientContexts;
};

//------------------------------------------------------------------------------
/**
*/
inline
bool
nRemoteServer::IsOpen() const
{
    return this->isOpen;
}

//------------------------------------------------------------------------------
/**
*/
inline
nRemoteServer::nClientContext::nClientContext(int cid, nRemoteServer* owner) :
    clientId(cid), refCwd(owner)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
nRemoteServer::nClientContext::~nClientContext()
{
	release_ref(refCwd);    
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nRemoteServer::nClientContext::GetClientId() const
{
    return this->clientId;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nRemoteServer::nClientContext::SetCwd(nRoot* obj)
{
    n_assert(obj);
    this->refCwd = obj;
}

//------------------------------------------------------------------------------
/**
    Get the client specific cwd object. If the object no longer exists,
    return 0.
*/
inline
nRoot*
nRemoteServer::nClientContext::GetCwd() const
{
    return this->refCwd.isvalid() ? this->refCwd.get() : 0;
}

//------------------------------------------------------------------------------
#endif




    
