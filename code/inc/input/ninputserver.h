#ifndef N_INPUTSERVER_H
#define N_INPUTSERVER_H
//-------------------------------------------------------------------
/**
    @class nInputServer
    
    @brief manages input events and mappings
*/
//-------------------------------------------------------------------
#ifndef N_TYPES_H
#include "kernel/ntypes.h"
#endif

#ifndef N_ROOT_H
#include "kernel/nroot.h"
#endif

#ifndef N_AUTOREF_H
#include "kernel/nautoref.h"
#endif

#ifndef N_INPUTEVENT_H
#include "input/ninputevent.h"
#endif

#ifndef N_INPUTSTATE_H
#include "input/ninputstate.h"
#endif

#undef N_DEFINES
#define N_DEFINES nInputServer
#include "kernel/ndefdllclass.h"

//------------------------------------------------------------------------------
class nScriptServer;
class nConServer;
class N_PUBLIC nInputServer : public nRoot 
{
public:
    /// constructor
    nInputServer();
    /// destructor
    virtual ~nInputServer();
    /// per frame trigger 
    virtual void Trigger(double time);
    /// create a new input event object
    virtual nInputEvent *NewEvent(void);
    /// release an input event object
    virtual void ReleaseEvent(nInputEvent* event);
    /// link input event object into global input event list
    virtual void LinkEvent(nInputEvent* event);
    /// unlink input event object from global input event list
    virtual void UnlinkEvent(nInputEvent* event);
    /// flush (clear) global input event list
    virtual void FlushEvents(void);
    /// return pointer to first input event in global input event list
    virtual nInputEvent *FirstEvent(void);
    /// return pointer to next event in global input event list
    virtual nInputEvent *NextEvent(nInputEvent *);
    /// begin mapping input events
    void BeginMap(void);
    /// map an input event to an input state
    bool Map(const char* event, const char* state);
    /// finish mapping input
    void EndMap();
    /// get an input state as slider value
    float GetSlider(const char* state);
    /// get an input state as button value
    bool GetButton(const char* state);
    /// start logging input
    void StartLogging();
    /// stop logging input
    void StopLogging(void);
    /// is currently logging input?
    bool IsLogging(void);
    /// set the long pressed time
    void SetLongPressedTime(float);
    /// get the long pressed time
    float GetLongPressedTime(void);
    /// set the double click time
    void SetDoubleClickTime(float);
    /// get the double click time
    float GetDoubleClickTime(void);
    /// set mouse sensitivity
    virtual void SetMouseFactor(float s);
    /// get mouse sensitivity
    virtual float GetMouseFactor() const;
    /// set invert mouse flag
    virtual void SetMouseInvert(bool b);
    /// get invert mouse flag
    virtual bool GetMouseInvert() const;
    /// call when input focus obtained
    virtual void ObtainFocus(void);
    /// call when input focus lost
    virtual void LoseFocus(void);
	/// flush entire input mapping
	virtual void FlushInput();

    static nKernelServer* kernelServer;

private:
    /// convert an input event string into an input event
    bool MapStrToEvent(const char* str, nInputEvent* event);
    /// return pointer to first matching event
    nInputEvent *FirstIdenticalEvent(nInputEvent *pattern);
    /// return pointer to next matching event
    nInputEvent *NextIdenticalEvent(nInputEvent *pattern, nInputEvent *cur);
    /// do actual input mapping
    void DoInputMapping(void);
    /// begin adding a script to execute
    void BeginScripts(void);
    /// add a script to execute
    void AddScript(const char* script);
    /// finish adding script
    void EndScripts(void);
    /// check if events are identical
    bool IsIdenticalEvent(nInputEvent* event0, nInputEvent* event1);    
    /// log a single event
    void LogSingleEvent(nInputEvent* event);
    /// log all events
    void LogEvents();
    /// find input state object by name
    nInputState *GetInputState(const char* state);
    /// add a named input state
    nInputState *AddInputState(const char* state);
    /// export the default keyboard
    void ExportDefaultKeyboard(void);
    /// export the default mouse
    void ExportDefaultMouse(void);
    /// build an input mapping name
    const char* BuildInputMappingName(const char* ieStr, const char* isStr, char* buf, int bufSize);

protected:
    enum 
    {
        N_MAXNUM_SCRIPTS = 16,              // max number of script cmds per frame
    };

    nAutoRef<nScriptServer> ref_ss;         // "/sys/servers/script"
    nAutoRef<nConServer>    ref_con;        // "/sys/servers/console"
    nRef<nRoot> ref_statedir;               // "/sys/share/input/states"
    nRef<nRoot> ref_inpdir;                 // "/sys/share/input"

    nList events;               // list on nInputEvents for this frame

    bool log_events;
    bool in_begin_map;

    nHashList im_list;          // list of nInputMapping's
    nHashList is_list;          // list of nInputState's

    int act_script;
    const char *script_array[N_MAXNUM_SCRIPTS];

    double long_pressed_time;
    double double_click_time;
    float mouseFactor;
    bool mouseInvert;
};

//------------------------------------------------------------------------------
/**
*/
inline
void
nInputServer::SetMouseFactor(float f)
{
    this->mouseFactor = f;
    n_printf("nInputServer::SetMouseFactor(%f)\n", f);
}

//------------------------------------------------------------------------------
/**
*/
inline
float
nInputServer::GetMouseFactor() const
{
    return this->mouseFactor;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nInputServer::SetMouseInvert(bool b)
{
    this->mouseInvert = b;
    n_printf("nInputServer::SetInvertMouse(%s)\n", b ? "true" : "false");
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
nInputServer::GetMouseInvert() const
{
    return this->mouseInvert;
}

//------------------------------------------------------------------------------
#endif
