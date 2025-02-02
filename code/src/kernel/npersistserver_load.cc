#define N_IMPLEMENTS nPersistServer
#define N_KERNEL
//------------------------------------------------------------------------------
//  npersistserver_load.cc
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "kernel/npersistserver.h"
#include "kernel/nkernelserver.h"
#include "kernel/nscriptserver.h"
#include "kernel/nfileserver2.h"
#include "kernel/nfile.h"
#include "util/npathstring.h"

//------------------------------------------------------------------------------
/**
    Scan starting 256 bytes of a file for a "$keyword:string$" and return
    keyword belonging to string.
*/
char *
nPersistServer::ReadEmbeddedString(const char *fname, const char *keyword, char *buf, int buf_size)
{
    char header_buf[256];
    int num_bytes;
    nFile* file = this->refFileServer->NewFileObject();
    n_assert(file);
    if (file->Open(fname, "r"))
    {
        num_bytes = file->Read(header_buf, sizeof(header_buf));
        file->Close();
        delete file;
        
        if (num_bytes > 0) {

            // suche nach $
            char *str = header_buf;
            char *end_str = str + num_bytes;
            while (str < end_str) 
            {
                if (*str == '$') 
                {					
                    char *end = 0;
                    *str++ = 0;
                    // liegt das Ende im Buffer?
                    if (end = strchr(str,'$')) 
                    {
                        *end++ = 0;
                        // Trennzeichen ':'
                        char* tmp = strchr(str,':');
                        if (tmp) 
                        {
                            *tmp++ = 0;
                            // korrektes Keyword?
                            if (strcmp(str,keyword)==0) 
                            {
                                n_strncpy2(buf, tmp, buf_size);
                                return buf;
                            }
                        } 
                        else 
                        {
                            kernelServer->Message("$...$ definition in object file broken!"); 
                            return 0;
                        }
                        // Scan-Pointer auf Ende des $..$ Blocks
                        str = end;
                    } 
                    else
                    {
                        kernelServer->Message("$...$ definition in object file outside first 256 bytes!");
                        return 0;
                    }            
                } 
                else 
                {
                    str++;
                }
            }
        }
    }
    else
    {
        delete file;
    }
    
    // $..$ Block nicht gefunden, oder File nicht lesbar
    return 0;
}

//------------------------------------------------------------------------------
/**
    Create a Nebula object from a persistent object file.

    10-Nov-98   floh    created
    30-Nov-98   floh    + wertet jetzt override_class aus
    10-Jan-99   floh    + Fehlermeldungen verbessert
    30-Jul-99   floh    + Override-Class gekillt
    20-Jan-00   floh    + rewritten for ref_ss
    28-Sep-00   floh    + PushCwd()/PopCwd()
*/
nRoot*
nPersistServer::LoadFoldedObject(const char *fname, const char *objName)
{
    nRoot *obj = 0;
    char parserBuf[128];
    char objBuf[128];
    const char* parserClass;
    const char* objClass;
    
    // read parser and object class meta data from file
    parserClass = this->ReadEmbeddedString(fname, "parser", parserBuf, sizeof(parserBuf));
    objClass    = this->ReadEmbeddedString(fname, "class", objBuf, sizeof(objBuf));
    if (!parserClass) 
    {
        return 0;
    }
    if (!objClass) 
    {
        return 0;
    }

    // create object and parse script
    obj = kernelServer->New(objClass, objName);
    if (obj) 
    {
        kernelServer->PushCwd(obj);
        const char* result;
        nScriptServer* loader = this->GetLoader(parserBuf);
        loader->RunScript(fname, result);
        kernelServer->PopCwd();
    } 
    return obj; 
}

//------------------------------------------------------------------------------
/**
    Frontend to load an object from a persistent object file.

    10-Nov-98   floh    created
    01-Dec-98   floh    im Test, ob das Objekt schon existiert,
                        wird jetzt einfach der RefCount erhoeht
    13-Jan-99   floh    + oops, unter Linux kann auch Directory per
                        fopen() geoeffnet werden, damit wollte ich
                        aber herausfinden, ob es sich um einen 
                        .n FILE oder ein .n DIR handelt, deshalb
                        jetzt Erkennung per nDir.
    08-Feb-99   floh    Im Folded Modus wird jetzt zuerst das
                        cwd auf die Diretory-Komponente des
                        Filenamens eingestellt.
    17-Feb-99   floh    Ooops, boeser Bug beim Laden von Einzelfiles...
                        zwar wurde auf das aktuelle Directory gewechselt,
                        dann aber trotzdem der komplette Pfadname
                        an den Lader uebergeben...
    29-Mar-99   floh    + kommt jetzt mit Assigns zurecht
    26-Apr-01   floh    + Bugfix: _loadUnfoldedObject() didn't use
                          mangled path
    30-Jul-02   floh    + many simplifications
    29-Jan-03   floh    + simplified by using nPathString
*/
nRoot*
nPersistServer::LoadObject(const char* fname)
{
    n_assert(fname);
    
    // isolate object name from path, object path can have 2 forms:
    //
    //  (1) xxx/blub.n/_main.n      -> a folded object
    //  (2) xxx/blub.n              -> an unfolded object
    //
    nPathString path(fname);
    path.ConvertBackslashes();
    nPathString objName = path.ExtractFileName();
    objName.StripExtension();

    // drop out if trying to load existing object
    nRoot *obj = kernelServer->Lookup(objName.c_str());
    if (obj)
    {        
        n_error("nPersistServer: trying to overwrite existing object '%s'!\n", obj->GetFullName().c_str());
        return 0;
    }
    else 
    {   
        // try to load object as folded object, if that fails try unfolded
        obj = this->LoadFoldedObject(fname, objName.c_str());
        if (!obj)
        {
            char unfoldedName[N_MAXPATH];
            sprintf(unfoldedName, "%s/_main.n", fname);
            obj = this->LoadFoldedObject(unfoldedName, objName.c_str());
            if (!obj)
            {
                // could'nt load object!
                //n_message("nPersistServer: Could not load object '%s'!\n", fname);
				n_printf("nPersistServer: Could not load object '%s'!\n", fname);
                return 0;
            }
        }
    }
    return obj;
}

//--------------------------------------------------------------------
//  ParseFile()
//  Wendet die Kommandos in einem Script-File auf ein Objekt
//  an. Bei dem File werden die "$class:" und "$parser:"
//  Statements ausgewertet um zu pruefen, ob das Objekt die
//  Kommandos verstehen kann, etc...
//  10-Jan-99   floh    created
//  20-Jan-00   floh    + rewritten for ref_ss
//  28-Sep-00   floh    + PushCwd()/PopCwd()
//  04-Apr-03   orpheus robbed from nfileserver
//--------------------------------------------------------------------
bool nPersistServer::ParseFile(nRoot *o, const char *fname)
{
    n_assert(o);
    n_assert(fname);    
    
    const char *parser_class, *obj_class;
    char parser_buf[128];
    char obj_buf[128];
    nClass *cl;
    
    // Parser-Klasse und Objekt-Klasse vom File lesen
    parser_class = ReadEmbeddedString(fname, "parser", parser_buf, sizeof(parser_buf));
    obj_class    = ReadEmbeddedString(fname, "class", obj_buf, sizeof(obj_buf));
    if (!parser_class) {
        n_printf("Could not read script parser class from file %s!\n", fname);
        return false;
    }
    if (!obj_class) {
        n_printf("Could not read class from file %s!\n", fname);
        return false;
    }
    
    // versteht das Objekt die Kommandos?
    cl = ks->FindClass(obj_class);
    if (cl && (o->IsA(cl))) 
	{
        // Backup vom Cwd
        const char* result;
        ks->PushCwd(o);
        this->GetLoader(parser_buf)->RunScript(fname, result);
        ks->PopCwd();
        return true;
    } 
	else 
	{
        n_printf("Object of class '%s' doesn't accept cmds of class '%s'\n",
                 o->GetClass()->GetName(), obj_class);
        return false;
    }    
}

//------------------------------------------------------------------------------
//  EOF
//------------------------------------------------------------------------------
