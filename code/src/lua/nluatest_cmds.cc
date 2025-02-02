#define N_IMPLEMENTS nLuaTest
//------------------------------------------------------------------------------
//  (c) 2003 Vadim Macagon
//------------------------------------------------------------------------------
#include "lua/nluatest.h"

static void n_testscriptcmds(void* slf, nCmd* cmd);
static void n_makezombies(void* slf, nCmd* cmd);

//------------------------------------------------------------------------------
/**
  @scriptclass
  nluatest

  @superclass
  nroot

  @classinfo
  Just a simple test thingie for lua server stuff.
*/
void
n_initcmds( nClass* clazz )
{
  clazz->BeginCmds();
  clazz->AddCmd( "v_testscriptcmds_v", 'TSC_', n_testscriptcmds );
  clazz->AddCmd( "v_makezombies_v", 'MZOM', n_makezombies );
  clazz->EndCmds();
}

//------------------------------------------------------------------------------
/**
  @cmd
  testscriptcmds

  @input
  v

  @output
  v

  @info
  Test calling of script-side commands from C++.
*/
static
void
n_testscriptcmds(void* slf, nCmd* cmd)
{
  nLuaTest* self = (nLuaTest*)slf;
  self->TestScriptCmds();
}

//------------------------------------------------------------------------------
/**
  @cmd
  makezombies

  @input
  v

  @output
  v

  @info
  Create some zombie thunks in the internal thunks store.
*/
static
void
n_makezombies(void* slf, nCmd* cmd)
{
  nLuaTest* self = (nLuaTest*)slf;
  self->MakeZombies();
}

//------------------------------------------------------------------------------
//  EOF
//------------------------------------------------------------------------------
