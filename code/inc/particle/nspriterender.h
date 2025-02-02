#ifndef N_SPRITERENDER_H
#define N_SPRITERENDER_H
//--------------------------------------------------------------------
/**
    @class nSpriteRender

    @brief render particles as viewer aligned sprites
*/
//--------------------------------------------------------------------
#ifndef N_REF_H
#include "kernel/nref.h"
#endif

#ifndef NMATH_MATH_H
#include "mathlib/nmath.h"
#endif

#ifndef N_PRENDER_H
#include "particle/nprender.h"
#endif

#ifndef N_DYNVERTEXBUFFER_H
#include "gfx/ndynvertexbuffer.h"
#endif

//--------------------------------------------------------------------
#undef N_DEFINES
#define N_DEFINES nSpriteRender
#include "kernel/ndefdllclass.h"

//--------------------------------------------------------------------
class nGfxServer;
class nMathServer;
class N_DLLCLASS nSpriteRender : public nPRender 
{
    nAutoRef<nGfxServer>  ref_gs;
    nAutoRef<nMathServer> ref_math;
    
    nVertexBuffer *cur_vb;
    int num_quads;              // number of quads which fit into the target vertex buffer
    int cur_quad;               // number of current quad in vbuffer

    float *coord;
    ulong *color;
    float *uv;
    int stride4;

    int bounce_key;
    float spinRef;              // the spin in "REFQUADS per second"
    float spinAccelRef;         // the spin accel in "REFQUADS per second"


    nDynVertexBuffer dyn_vb;
    nRef<nIndexBuffer> ref_ibuf;

public:
    // reference vertices (for spin)
    enum
    {
        REFQUADS = 64
    };
    static vector3 refVertices[REFQUADS * 4];
    static vector3 tformedVertices[REFQUADS * 4];
    
    static nKernelServer* kernelServer;

    nSpriteRender();
	virtual void Initialize();
    virtual ~nSpriteRender();
    virtual bool Attach(nSceneGraph2* sceneGraph);
    virtual void Compute(nSceneGraph2* sceneGraph);

private:
    void init_buffers(void);
    void begin_render(nPixelShader *, nTextureArray *);
    void swap_render(void);
    void end_render(void);
    void render_particles(nSceneGraph2*, float);
};
//--------------------------------------------------------------------
#endif
