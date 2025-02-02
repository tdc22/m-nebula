#define N_IMPLEMENTS nGfxServer
//-------------------------------------------------------------------
//  gfx/ngfx_dispatch.cc
//  This file was machine generated.
//  (C) 2000 A.Weissflog/A.Flemming
//-------------------------------------------------------------------
#include "gfx/ngfxserver.h"
#include "kernel/nstr2enum.h"

static struct {	const char* str; nClearBufType en; } clean_table[] = {
	{"color",	N_COLOR_BUFFER},
	{"depth",	N_DEPTH_BUFFER},
	{"stencil",	N_STENCIL_BUFFER},
	{"accum",	N_ACCUM_BUFFER}
};


static void n_setdisplaymode(void *, nCmd *);
static void n_getdisplaymode(void *, nCmd *);
static void n_resize(void *, nCmd *);
static void n_opendisplay(void *, nCmd *);
static void n_closedisplay(void *, nCmd *);
static void n_setviewvolume(void *, nCmd *);
static void n_getviewvolume(void *, nCmd *);
static void n_setperspective(void *, nCmd *);
static void n_getperspective(void *, nCmd *);
static void n_setfov(void *, nCmd *);
static void n_getfov(void *, nCmd *);
static void n_setclearcolor(void *, nCmd *);
static void n_getclearcolor(void *, nCmd *);
static void n_setwindowmode(void *, nCmd *);
static void n_getwindowmode(void *, nCmd *);
static void n_screenshot(void *, nCmd *);
static void n_scaledscreenshot(void *, nCmd *);
static void n_begincursors(void*, nCmd*);
static void n_setcursor(void*, nCmd*);
static void n_endcursors(void*, nCmd*);
static void n_showcursor(void*, nCmd*);
static void n_hidecursor(void*, nCmd*);
static void n_setcurrentcursor(void*, nCmd*);
static void n_getcurrentcursor(void*, nCmd*);
static void n_setnormalize(void*, nCmd*);
static void n_getnormalize(void*, nCmd*);
static void n_setortho(void*, nCmd*);
static void n_getortho(void*, nCmd*);
static void n_setantialiasing(void*, nCmd*);
static void n_getantialiasing(void*, nCmd*);
static void n_setcursorpos(void*, nCmd*);
static void n_setclearbuffer(void*, nCmd*);
static void n_getclearbuffer(void*, nCmd*);

//------------------------------------------------------------------------------
/**
    @scriptclass
    ngfxserver

    @superclass
    nroot

    @classinfo
    Superclass for all graphic server classes, gfx server objects
    generally reside under the name "/sys/servers/gfx".
*/
void n_initcmds(nClass *cl)
{
    cl->BeginCmds();
    cl->AddCmd("b_setdisplaymode_s",        'SDPM', n_setdisplaymode);
    cl->AddCmd("s_getdisplaymode_v",        'GDPM', n_getdisplaymode);
	cl->AddCmd("v_resize_iiii",				'RESW',	n_resize);
    cl->AddCmd("b_opendisplay_v",           'ODSP', n_opendisplay);
    cl->AddCmd("v_closedisplay_v",          'CDSP', n_closedisplay);
    cl->AddCmd("v_setviewvolume_ffffff",    'SVWV', n_setviewvolume);
    cl->AddCmd("ffffff_getviewvolume_v",    'GVWV', n_getviewvolume);
    cl->AddCmd("v_setperspective_ffff",     'SPER', n_setperspective);
    cl->AddCmd("ffff_getperspective_v",     'GPER', n_getperspective);
    cl->AddCmd("v_setfov_f",                'SFOV', n_setfov);
    cl->AddCmd("f_getfov_v",                'GFOV', n_getfov);
    cl->AddCmd("v_setclearcolor_ffff",      'SCLC', n_setclearcolor);
    cl->AddCmd("ffff_getclearcolor_v",      'GCLC', n_getclearcolor);

    cl->AddCmd("v_setwindowmode_sb",        'SWNM', n_setwindowmode);
	cl->AddCmd("b_getwindowmode_s",			'GWNM', n_getwindowmode);
    cl->AddCmd("b_screenshot_s",            'SSHT', n_screenshot);
    cl->AddCmd("b_scaledscreenshot_sii",    'SCLS', n_scaledscreenshot);
    cl->AddCmd("v_begincursors_i",          'BCUR', n_begincursors);
    cl->AddCmd("v_setcursor_isii",          'SCUR', n_setcursor);
    cl->AddCmd("v_endcursors_v",            'ECUR', n_endcursors);
    cl->AddCmd("v_showcursor_v",            'SHCR', n_showcursor);
    cl->AddCmd("v_hidecursor_v",            'HCUR', n_hidecursor);
    cl->AddCmd("v_setcurrentcursor_ib",     'SCCR', n_setcurrentcursor);
    cl->AddCmd("i_getcurrentcursor_v",      'GCCR', n_getcurrentcursor);
    cl->AddCmd("v_setnormalize_b",          'STNR', n_setnormalize);
	cl->AddCmd("b_getnormalize_v",          'GTNR', n_getnormalize);
	cl->AddCmd("v_setortho_b",				'STOR', n_setortho);
	cl->AddCmd("b_getortho_v",				'GTOR', n_getortho);
	cl->AddCmd("b_setantialiasing_b",		'STAA', n_setantialiasing);
	cl->AddCmd("b_getantialiasing_v",		'GTAA', n_getantialiasing);
	cl->AddCmd("v_setcursorpos_ff",			'SCPS', n_setcursorpos);
	cl->AddCmd("v_setclearbuffer_sb",       'SCBF', n_setclearbuffer);
	cl->AddCmd("b_getclearbuffer_s",        'GCBF', n_getclearbuffer);
	
    cl->EndCmds();
}

//------------------------------------------------------------------------------
/**
    @cmd
    setdisplaymode

    @input
    s (DisplayMode)

    @output
    b (Success)

    @info
    Define the display mode that should be used at the next
    'opendisplay'. The display mode string consists of any
    number of tags of the form 'tag(value)', which are separated
    by a '-'. The string may not contain spaces.
    The following tags are defined:
    
    dev(id)
        Define the rendering device to be used. This
        may be a number from 0..n if there is more then
        one rendering device in the system. The default
        rendering device is always 0. The valid rendering
        devices can be examined under the path
        '/sys/share/display'.
    type(win|full)
        Specifies whether windowed mode 'type(win)'
        or fullscreen mode 'type(full)'.
    w(width), h(height), bpp(bitsperpixel)
        Define width, height and depth of display mode.
    
    If there is no match for the selected display mode,
    'opendisplay' will try to find a display mode that
    is as close as possible to the mode descriptor string.
    Examples:
      .setdisplaymode "dev(1)-type(full)-w(800)-h(600)-bpp(32)"
      .setdisplaymode "type(win)-w(800)"
      .setdisplaymode "type(full)"
*/
static void n_setdisplaymode(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    cmd->Out()->SetB(self->SetDisplayMode(cmd->In()->GetS()));
}

//------------------------------------------------------------------------------
/**
    @cmd
    getdisplaymode

    @input
    v

    @output
    s (DisplayMode)

    @info
    Return the currently active display mode descriptor string.
    This may be different then the one set by 'setdisplaymode'
    if the wanted display mode was not available and a different
    mode had to be selected.
*/
static void n_getdisplaymode(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    cmd->Out()->SetS(self->GetDisplayMode());
}

//------------------------------------------------------------------------------
/**
    @cmd
    opendisplay

    @input
    v

    @output
    b (Success)

    @info
    Open the display. Switch to fullscreen mode if necessary.
    Changes made with 'setdisplaymode' only take effect after
    the display has been (re-)opened.
*/
static void n_opendisplay(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    cmd->Out()->SetB(self->OpenDisplay());
}

//------------------------------------------------------------------------------
/**
    @cmd
    closedisplay

    @input
    v

    @output
    v

    @info
    Close the display.
*/
static void n_closedisplay(void *o, nCmd *)
{
    nGfxServer *self = (nGfxServer *) o;
    self->CloseDisplay();
}

//------------------------------------------------------------------------------
/**
    @cmd
    setviewvolume

    @input
    f (MinX), f (MaxX), f (MinY), f (MaxY), f (MinZ), f (MaxZ)

    @output
    v

    @info
    Define the view volume, which is a pyramid of 6 planes.
    MinZ and MaxZ define the front and back clipping plane.
    MinX, MaxX, MinY, MaxY describe a rectangle on the
    front clipping plane where the side clipping planes
    cut through the front clipping plane. The default
    view volume is:
        MinX = -0.1
        MaxX = +0.1
        MinY = -0.1
        MaxY = +0.1
        MinZ = +0.1
        MaxZ = +25000.0
    ...which defines a 90 degree angle of view.
    
    WARNING: If you intend to make use of GetPerspective,
    SetFOV, GetFOV, please ensure that MinX == -MaxX and
    MinY == -MaxY. If you break these rules you're likely
    to get bogus values from the aformentioned methods.
*/
static void n_setviewvolume(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float minx = cmd->In()->GetF();
    float maxx = cmd->In()->GetF();
    float miny = cmd->In()->GetF();
    float maxy = cmd->In()->GetF();
    float minz = cmd->In()->GetF();
    float maxz = cmd->In()->GetF();
    self->SetViewVolume(minx,maxx,miny,maxy,minz,maxz);
}

//------------------------------------------------------------------------------
/**
    @cmd
    getviewvolume

    @input
    v

    @output
    f (MinX), f (MaxX), f (MinY), f (MaxY), f (MinZ), f (MaxZ)

    @info
    Return the values representing the current view volume.
*/
static void n_getviewvolume(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float minx,maxx,miny,maxy,minz,maxz;
    self->GetViewVolume(minx,maxx,miny,maxy,minz,maxz);
    cmd->Out()->SetF(minx);
    cmd->Out()->SetF(maxx);
    cmd->Out()->SetF(miny);
    cmd->Out()->SetF(maxy);
    cmd->Out()->SetF(minz);
    cmd->Out()->SetF(maxz);
}

//------------------------------------------------------------------------------
/**
    @cmd
    setperspective

    @input
    f (fovDegrees), f (aspectRatio), f (near), f (far)

    @output
    v

    @info
    Provides a simpler way to define the view volume. 
    
    (fovDegrees)
      The vertical field of view.
      Valid range: 0-180 degrees (exclusive).
      If value is outside this range, fov is set to 
      the default 90 degrees (just to save you the trouble 
      of sitting there trying to figure out why there is 
      nothing but black screen in front of you).
      
    (aspectRatio) 
      Display width divided by the display height.
    
    (near)
      Distance to near plane of frustum from origin.
      
    (far)
      Distance to far plane of frustum from origin.
    
    See OpenGL docs for more info on gluPerspective.
*/
static void n_setperspective(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float fovDegrees = cmd->In()->GetF();
    float aspect = cmd->In()->GetF();
    float zNear = cmd->In()->GetF();
    float zFar = cmd->In()->GetF();
    self->SetPerspective(fovDegrees,aspect,zNear,zFar);
}

//------------------------------------------------------------------------------
/**
    @cmd
    getperspective

    @input
    v

    @output
    f (fovDegrees), f (aspectRatio), f (near), f (far)

    @info
    Returns the values represeting the current
    perspective projection.
*/
static void n_getperspective(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float fovDegrees, aspect, zNear, zFar;
    self->GetPerspective(fovDegrees,aspect,zNear,zFar);
    cmd->Out()->SetF(fovDegrees);
    cmd->Out()->SetF(aspect);
    cmd->Out()->SetF(zNear);
    cmd->Out()->SetF(zFar);
}

//------------------------------------------------------------------------------
/**
    @cmd
    setfov

    @input
    f (fovDegrees)

    @output
    v

    @info
    Changes the current field of view, valid range
    for fovDegrees is 0-180 degrees (exclusive). 
    
    Use this if you don't need to change any of the 
    other parameters passed to SetPerspective 
    (ie. aspect ratio, near & far planes)
*/
static void n_setfov(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    self->SetFOV(cmd->In()->GetF());
}

//------------------------------------------------------------------------------
/**
    @cmd
    getfov

    @input
    f (fovDegrees)

    @output
    v

    @info
    Returns the current field of view (in degrees).
*/
static void n_getfov(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    cmd->Out()->SetF(self->GetFOV());
}

//------------------------------------------------------------------------------
/**
    @cmd
    setclearcolor

    @input
    f (Red), f (Green), f (Blue), f (Alpha)

    @output
    v

    @info
    Set background color.
*/
static void n_setclearcolor(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float r,g,b,a;
    r = cmd->In()->GetF();
    g = cmd->In()->GetF();
    b = cmd->In()->GetF();
    a = cmd->In()->GetF();
    self->SetClearColor(r,g,b,a);
}

//------------------------------------------------------------------------------
/**
    @cmd
    getclearcolor

    @input
    v

    @output
    f (Red), f (Green), f (Blue), f (Alpha)

    @info
    Return current background color.
*/
static void n_getclearcolor(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    float r,g,b,a;
    self->GetClearColor(r,g,b,a);
    cmd->Out()->SetF(r);
    cmd->Out()->SetF(g);
    cmd->Out()->SetF(b);
    cmd->Out()->SetF(a);
}

//------------------------------------------------------------------------------
/**
    @cmd
    setwindowmode

    @input
    s (active | hidden | topmost | framed)
	b set mode	on/off

    @output
    v

    @info
    Put window into one of several modes. May or may not be 
    implemented by actual gfx server subclasses.

    active      - activate the app window, so that it receives 
                  keyboard focus
    hidden      - hide or show the app window    
	topmost		- set or unset window to be topmost
	framed		- set or remove window caption	
*/
static void n_setwindowmode(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    const char *s = cmd->In()->GetS();
	bool b = cmd->In()->GetB();

    nGfxServer::nWindowMode wm = nGfxServer::N_WINDOWMODE_UNDEFINED;

    if (0 == strcmp(s, "active"))
    {
        wm = nGfxServer::N_WINDOWMODE_ACTIVE;
    }    
    else if (0 == strcmp(s, "hidden"))
    {
        wm = nGfxServer::N_WINDOWMODE_HIDDEN;
    }
	else if (0 == strcmp(s, "topmost"))
	{
		wm = nGfxServer::N_WINDOWMODE_TOPMOST;
	}
	else if (0 == strcmp(s, "framed"))
	{
		wm = nGfxServer::N_WINDOWMODE_FRAMED;
	}
    else
    {
        n_printf("nGfxServer::SetWindowMode() illegal window mode '%s'\n", s);
    }
	if (b)
		self->SetWindowMode(wm);
	else
		self->UnsetWindowMode(wm);
}


//------------------------------------------------------------------------------
/**
	@cmd
	getwindowmode

	@input
	s (active | hidden | topmost | framed)

	@output
	b - is asked mode on/off

	@info
	active      - activate the app window, so that it receives 	keyboard focus
	hidden      - hide or show the app window    
	topmost		- set or unset window to be topmost
	framed		- set or remove window caption	
*/
static void n_getwindowmode(void *o, nCmd *cmd)
{
	nGfxServer *self = (nGfxServer *) o;
	const char *s = cmd->In()->GetS();

	nGfxServer::nWindowMode wm = nGfxServer::N_WINDOWMODE_UNDEFINED;

	if (0 == strcmp(s, "active"))
	{
		wm = nGfxServer::N_WINDOWMODE_ACTIVE;
	}    
	else if (0 == strcmp(s, "hidden"))
	{
		wm = nGfxServer::N_WINDOWMODE_HIDDEN;
	}
	else if (0 == strcmp(s, "topmost"))
	{
		wm = nGfxServer::N_WINDOWMODE_TOPMOST;
	}
	else if (0 == strcmp(s, "framed"))
	{
		wm = nGfxServer::N_WINDOWMODE_FRAMED;
	}
	else
	{
		n_printf("nGfxServer::SetWindowMode() illegal window mode '%s'\n", s);
	}
	cmd->Out()->SetB(self->GetWindowMode(wm));
}

//------------------------------------------------------------------------------
/**
    @cmd
    screenshot

    @input
    s (FileName)

    @output
    v

    @info
    Generate a screenshot under the given filename.
    A 24 bpp BMP file will be created.
*/
static void n_screenshot(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    cmd->Out()->SetB(self->Screenshot(cmd->In()->GetS()));
}

//------------------------------------------------------------------------------
/**
    @cmd
    scaledscreenshot

    @input
    s (FileName), i (Width), i (Height)

    @output
    v

    @info
    Generate a screenshot scaled to a given size.
*/
static void n_scaledscreenshot(void *o, nCmd *cmd)
{
    nGfxServer *self = (nGfxServer *) o;
    const char *n = cmd->In()->GetS();
    int w = cmd->In()->GetI();
    int h = cmd->In()->GetI();
    cmd->Out()->SetB(self->ScaledScreenshot(n,w,h));
}

//------------------------------------------------------------------------------
/**
    @cmd
    begincursors
    @input
    i(NumCursors)
    @output
    v
    @info
    Start defining mouse cursors. Expects the number of cursors that follow.
*/
static
void
n_begincursors(void* slf, nCmd* cmd)
{
    nGfxServer* self = (nGfxServer*) slf;
    self->BeginCursors(cmd->In()->GetI());
}

//------------------------------------------------------------------------------
/**
    @cmd
    setcursors
    @input
    i(Index), s(ImagePath), i(HotspotX), i(HotspotY)
    @output
    v
    @info
    Define a cursor by giving the path to an image file, and a x/y hotspot
    position.
*/
static
void
n_setcursor(void* slf, nCmd* cmd)
{
    nGfxServer* self = (nGfxServer*) slf;
    int i0         = cmd->In()->GetI();
    const char* s0 = cmd->In()->GetS();
    int i1         = cmd->In()->GetI();
    int i2         = cmd->In()->GetI();
    self->SetCursor(i0, s0, i1, i2);
}

//------------------------------------------------------------------------------
/**
    @cmd
    endcursors
    @input
    v
    @output
    v
    @info
    Finish defining cursor images.
*/
static
void
n_endcursors(void* slf, nCmd* /*cmd*/)
{
    nGfxServer* self = (nGfxServer*) slf;
    self->EndCursors();
}

//------------------------------------------------------------------------------
/**
    @cmd
    showcursor
    @input
    v
    @output
    v
    @info
    Activate and show a mouse cursor defined by its index.
*/
static
void
n_showcursor(void* slf, nCmd* /*cmd*/)
{
    nGfxServer* self = (nGfxServer*) slf;
    self->SetCursorShown(true);
}

//------------------------------------------------------------------------------
/**
    @cmd
    hidecursor
    @input
    v
    @output
    v
    @info
    Hide the currently shown cursor.
*/
static
void
n_hidecursor(void* slf, nCmd* /*cmd*/)
{
    nGfxServer* self = (nGfxServer*) slf;
    self->SetCursorShown(false);
}

//------------------------------------------------------------------------------
/**
    @cmd
    setcurrentcursor
    @input
    i(CursorIndex), b(ShowCursor)
    @output
    v
    @info
    Select the current custom cursor by its index.
*/
static
void
n_setcurrentcursor(void* slf, nCmd* cmd)
{
    nGfxServer* self = (nGfxServer*) slf;
    int index = cmd->In()->GetI();
    bool show = cmd->In()->GetB();
    self->SetCurrentCursor(index, show);

}

//------------------------------------------------------------------------------
/**
    @cmd
    getcurrentcursor
    @input
    v
    @output
    i(CursorIndex)
    @info
    Get the currently selected cursor.
*/
static
void
n_getcurrentcursor(void* slf, nCmd* cmd)
{
    nGfxServer* self = (nGfxServer*) slf;
    cmd->Out()->SetI(self->GetCurrentCursor());
}

//------------------------------------------------------------------------------
/**
    @cmd
    setnormalize
    @input
    b - set auto normalization of normals on/off
    @output
    none
    @info
    controls auto normalization of normals
    
*/
static
void
n_setnormalize(void* slf, nCmd* cmd) {
    nGfxServer* self = (nGfxServer*) slf;
	self->SetAutoNormalize(cmd->In()->GetB());
}
    
//------------------------------------------------------------------------------
/**
    @cmd
    getnormalize
    @input
	none    
    @output
	b - get auto normalization of normals on/off    
    @info
    controls auto normalization of normals
    
*/
static
void
n_getnormalize(void* slf, nCmd* cmd) {
    nGfxServer* self = (nGfxServer*) slf;
	cmd->Out()->SetB(self->GetAutoNormalize());
}

//------------------------------------------------------------------------------
/**
    @cmd
    setortho
    @input
    b - set orthogonal view
    @output
    none
    @info
    controls orthogonal/perspective view
    
*/
static
void
n_setortho(void* slf, nCmd* cmd) {
    nGfxServer* self = (nGfxServer*) slf;
	self->SetOrtho(cmd->In()->GetB());
}
    
//------------------------------------------------------------------------------
/**
    @cmd
    getortho
    @input
	none    
    @output
	b - true means ortho view false perspective
    @info
    check orthogonal/perspective view    
*/
static
void
n_getortho(void* slf, nCmd* cmd) {
    nGfxServer* self = (nGfxServer*) slf;
	cmd->Out()->SetB(self->GetOrtho());
}


//------------------------------------------------------------------------------
/**
	@cmd
	setantialiasing
	@input
	b - set antialiasing on/off
	@output
	none
	@info
	controls antialiasing, if antialiasing is suported by HW
*/
static
void
n_setantialiasing(void* slf, nCmd* cmd) {
	nGfxServer* self = (nGfxServer*) slf;
	cmd->Out()->SetB(self->SetAA(cmd->In()->GetB()));
}

//------------------------------------------------------------------------------
/**
	@cmd
	getantialiasing
	@input
	none    
	@output
	b - true means antialiasing is on
	@info
	check antialiasing state
*/
static
void
n_getantialiasing(void* slf, nCmd* cmd) {
	nGfxServer* self = (nGfxServer*) slf;
	cmd->Out()->SetB(self->GetAA());
}

//------------------------------------------------------------------------------
/** 
	@cmd
	resize

	@input
	i i - x, y - top left coordinates of the window 
	i i - w, h - width and height of the window 

	@output
	v

	@info
	Resize the window to x, y, w, h
*/
static void n_resize(void* o, nCmd* cmd)
{
	nGfxServer* self = (nGfxServer*)o;
	int x = cmd->In()->GetI();
	int y = cmd->In()->GetI();
	int w = cmd->In()->GetI();
	int h = cmd->In()->GetI();

	self->Resize(x, y, w, h);
}

//------------------------------------------------------------------------------
/** 
	@cmd
	setcursorpos

	@input
	f f - x, y of mice pointer relative to upper left corner	

	@output
	v

	@info
	Position mice pointer relative to upper left corner	
*/
static void n_setcursorpos(void* o, nCmd* cmd)
{
	nGfxServer* self = (nGfxServer*)o;
	float x = cmd->In()->GetF();
	float y = cmd->In()->GetF();
	

	self->SetMousePos(x, y);
}

//------------------------------------------------------------------------------
/** 
	@cmd
	setclearbuffer

	@input
	s b  - name of buffer and flag to clear or not
		possible buffer names are:
		color
		depth
		stencil
		accum	

	@output
	v

	@info
	Set/unset clear flag for specified buffers
*/
static void n_setclearbuffer(void* o, nCmd* cmd)
{
	nGfxServer* self = (nGfxServer*)o;
	nClearBufType eCmd;
	if (str2enum(cmd->In()->GetS(), clean_table, sizeof(clean_table), eCmd))
	{
		self->SetClearBuffer(eCmd, cmd->In()->GetB());
	} 
	else
	{
		cmd->Rewind();
		n_printf("Incorrect buffer name %s\n", cmd->In()->GetS());		
	}
}

//------------------------------------------------------------------------------
/** 
	@cmd
	setclearbuffer

	@input
	s - name of buffer and flag to clear or not
		possible buffer names are:
		color
		depth
		stencil
		accum	

	@output
	b - is clear flag set

	@info
	Get clear flag for specified buffers
*/
static void n_getclearbuffer(void* o, nCmd* cmd)
{
	nGfxServer* self = (nGfxServer*)o;
	nClearBufType eCmd;
	if (str2enum(cmd->In()->GetS(), clean_table, sizeof(clean_table), eCmd))
	{
		cmd->Out()->SetB(self->GetClearBuffer(eCmd));
	}
}
//-------------------------------------------------------------------
//  EOF
//-------------------------------------------------------------------
