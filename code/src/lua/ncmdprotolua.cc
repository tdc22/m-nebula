#define N_IMPLEMENTS nCmdProtoLua
//------------------------------------------------------------------------------
//  (c) 2003 Vadim Macagon
//
//  nCmdProtoLua is licensed under the terms of the Nebula License.
//------------------------------------------------------------------------------
#include "lua/ncmdprotolua.h"
#include "lua/nluaserver.h"

//--------------------------------------------------------------------
/**
  @brief Constructor.
  @param protoDef [in] Blue print string.
*/
nCmdProtoLua::nCmdProtoLua(const char* protoDef)
  : nCmdProto(protoDef)
{
}

//--------------------------------------------------------------------
/**
  @brief Constructor.
*/
nCmdProtoLua::nCmdProtoLua(nCmdProtoLua* cp)
  : nCmdProto((nCmdProto*)cp)
{
}

//--------------------------------------------------------------------
/**
  @brief Execute a script command on the provided object.
*/
bool nCmdProtoLua::Dispatch(void* obj, nCmd* cmd)
{
    lua_State* L = nLuaServer::Instance->GetContext();
    
    int top = lua_gettop(L);
        
    // find the thunk that corresponds to the object
    // use the object pointer as a key into thunks table
    lua_pushstring(L, nLuaServer::Instance->thunkStoreName.c_str());
    lua_rawget(L, LUA_GLOBALSINDEX);
    n_assert((0 == lua_isnil(L, -1)) && "_nebthunks table not found!");
    lua_pushlightuserdata(L, obj);
    lua_gettable(L, -2);
    if (0 == lua_istable(L, -1)) // thunk not found?
    {
        //char buf[N_MAXPATH];
        //n_message("No thunk found for %s\n", 
        //          ((nRoot*)obj)->GetFullName(buf, sizeof(buf)));
        // bail out - no script-side script commands were defined
        return false;
    }
        
    // at this point the correct thunk is on top of the stack
    // find the function of interest
    lua_pushstring(L, cmd->GetProto()->GetName());
    lua_rawget(L, -2);
    if (0 == lua_isfunction(L, -1)) // function doesn't exist?
        return false;
    
    // shift thunk to be the first arg to the function
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    // put cmd in-args on the stack
    nLuaServer::InArgsToStack(L, cmd);
    // call the function
    if (0 != lua_pcall(L, cmd->GetNumInArgs() + 1, cmd->GetNumOutArgs(), 0))
    {
        // error occured, display it        
        n_error("LUA encountered an error (%s) while trying to call %s on %s",
                lua_tostring(L, -1), cmd->GetProto()->GetName(),
                ((nRoot*)obj)->GetFullName().c_str());
    }
    // put out-args into the cmd
    nLuaServer::StackToOutArgs(L, cmd);
    
    lua_settop(L, top);    
    return true;
}

//--------------------------------------------------------------------
//  EOF
//--------------------------------------------------------------------
