#ifndef N_ANIMNODE_H
#define N_ANIMNODE_H
//-------------------------------------------------------------------
/**
    @class nAnimNode
    @ingroup NebulaVisnodeModule
    @brief nAnimNode is the superclass of nodes which control animation.

    The animnode class administers some attributes shared by all
    animating node classes, for the purposes of animating or
    manipulating other nodes.
*/
//-------------------------------------------------------------------
#ifndef N_VISNODE_H
#include "node/nvisnode.h"
#endif

#ifndef N_AUTOREF_H
#include "kernel/nautoref.h"
#endif

#ifndef N_MATH_H
#include "mathlib/nmath.h"
#endif
//-------------------------------------------------------------------
#undef N_DEFINES
#define N_DEFINES nAnimNode
#include "kernel/ndefdllclass.h"
//-------------------------------------------------------------------
//  allgemeines Keyframe-Objekt mit Objekt-Pointer
//-------------------------------------------------------------------
class nObjectKey {
public:
    float t;
    nVisNode *o;
    nObjectKey() {
        t = 0.0f;
        o = NULL;
    };
    void Set(float _t, nVisNode *_o) {
        t = _t;
        o = _o;
    };
};
//-------------------------------------------------------------------
enum nRepType 
{
    N_REPTYPE_ONESHOT,
    N_REPTYPE_LOOP,
	N_REPTYPE_OSCILLATE,
};


//-------------------------------------------------------------------
class N_PUBLIC nAnimNode : public nVisNode {
public:
    /// constructor
    nAnimNode();
    /// destructor
    virtual ~nAnimNode();
    /// persistency
    virtual bool SaveCmds(nPersistServer *);
    /// allocate required channels
    virtual void AttachChannels(nChannelSet*);
    /// set animation repetition type
    void SetRepType(nRepType rType);
    /// get animation repetition type
    nRepType GetRepType();
    /// set animation channel name (default == "time")
    void SetChannel(const char *chnName);
    /// get animation channel name
    const char *GetChannel();
    /// set time scale
    void SetScale(float tScale);
    /// get time scale
    float GetScale();	
	/// Compute time according to reptype
	float ComputeTime(float t, float min_t, float max_t) const;

    static nKernelServer* kernelServer;

protected:
    nRepType repType;
    float scale;
    int localChannelIndex;
    char channelName[16];
};

//------------------------------------------------------------------------------
/**
    Set the repetition type of the animation. This can be N_REPTYPE_ONESHOT
    or N_REPTYPE_LOOP. The default is N_REPTYPE_LOOP.

    @param  rType       the repetition type
*/
inline
void
nAnimNode::SetRepType(nRepType rType)
{
    this->repType = rType;
}

//------------------------------------------------------------------------------
/**
    Get the repetition type for the animation.

    @return             the repetition type
*/
inline
nRepType
nAnimNode::GetRepType()
{
    return this->repType;
}

//------------------------------------------------------------------------------
/**
    Compute time according to reptype
    @return             time
*/
inline
float nAnimNode::ComputeTime(float t, float min_t, float max_t) const
{
	switch (this->repType)
	{
	case N_REPTYPE_LOOP:			
		t = t - (n_floor(t / max_t) * max_t);
		break;
	case N_REPTYPE_OSCILLATE:			
		{				
			float tr = n_floor(t / max_t);
			float u = (float)(n_ftol(tr)&0x1);
			t = t - (tr * max_t);
			t = (max_t - t)*u + t*(1.0f - u);
		}			
		break;
	}			

    // clamp time to range			
	float correction = (max_t - min_t) / 10000.0f;
    if      (t < min_t)  t = min_t;
    else if (t >= max_t) t = max_t - correction;
	return t;
}
//------------------------------------------------------------------------------
/**
    Set the animation channel, from which the animation should get its 
    "time info". This defaults to a channel named "time", which Nebula
    feeds with the current uptime of Nebula. The channel must be
    1-dimensional.

    @param  chnName     the channel name
*/
inline
void
nAnimNode::SetChannel(const char* chnName)
{
    n_assert(chnName);
    n_strncpy2(this->channelName, chnName, sizeof(this->channelName));
    this->NotifyChannelSetDirty();
}

//------------------------------------------------------------------------------
/**
    Return the current animation channel name.

    @return         the channel name
*/
inline
const char*
nAnimNode::GetChannel()
{
    return this->channelName;
}

//------------------------------------------------------------------------------
/**
    Set the time scale, can be used to accelerate or decelerate the
    animation.

    @param      tScale      the time scale, defaults to 1.0f
*/
inline
void
nAnimNode::SetScale(float tScale)
{
    n_assert(tScale != 0.0f);
    this->scale = tScale;
}

//------------------------------------------------------------------------------
/**
    Get the current time scale.
    
    @return     the current time scale
*/
inline
float
nAnimNode::GetScale()
{
    return this->scale;
}

//-------------------------------------------------------------------
#endif
