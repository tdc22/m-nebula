/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef _ODE_COLLISION_SPACE_H_
#define _ODE_COLLISION_SPACE_H_

#include <ode/common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dContactGeom;

/**
 * @brief User callback for geom-geom collision testing.
 *
 * @param data The user data object, as passed to dSpaceCollide.
 * @param o1   The first geom being tested.
 * @param o2   The second geom being test.
 *
 * @remarks The callback function can call dCollide on o1 and o2 to generate
 * contact points between each pair. Then these contact points may be added
 * to the simulation as contact joints. The user's callback function can of
 * course chose not to call dCollide for any pair, e.g. if the user decides
 * that those pairs should not interact.
 *
 * @ingroup collide
 */
typedef void dNearCallback (void *data, dGeomID o1, dGeomID o2);


ODE_API dSpaceID dSimpleSpaceCreate (dSpaceID space);
ODE_API dSpaceID dHashSpaceCreate (dSpaceID space);
ODE_API dSpaceID dQuadTreeSpaceCreate (dSpaceID space, dVector3 Center, dVector3 Extents, int Depth);


// SAP
// Order XZY or ZXY usually works best, if your Y is up.
#define dSAP_AXES_XYZ  ((0)|(1<<2)|(2<<4))
#define dSAP_AXES_XZY  ((0)|(2<<2)|(1<<4))
#define dSAP_AXES_YXZ  ((1)|(0<<2)|(2<<4))
#define dSAP_AXES_YZX  ((1)|(2<<2)|(0<<4))
#define dSAP_AXES_ZXY  ((2)|(0<<2)|(1<<4))
#define dSAP_AXES_ZYX  ((2)|(1<<2)|(0<<4))

ODE_API dSpaceID dSweepAndPruneSpaceCreate( dSpaceID space, int axisorder );



ODE_API void dSpaceDestroy (dSpaceID);

ODE_API void dHashSpaceSetLevels (dSpaceID space, int minlevel, int maxlevel);
ODE_API void dHashSpaceGetLevels (dSpaceID space, int *minlevel, int *maxlevel);

ODE_API void dSpaceSetCleanup (dSpaceID space, int mode);
ODE_API int dSpaceGetCleanup (dSpaceID space);

ODE_API void dSpaceAdd (dSpaceID, dGeomID);
ODE_API void dSpaceRemove (dSpaceID, dGeomID);
ODE_API int dSpaceQuery (dSpaceID, dGeomID);
ODE_API void dSpaceClean (dSpaceID);
ODE_API int dSpaceGetNumGeoms (dSpaceID);
ODE_API dGeomID dSpaceGetGeom (dSpaceID, int i);

/**
 * @brief Given a space, this returns its class.
 *
 * The ODE classes are:
 *  @li dSimpleSpaceClass
 *  @li dHashSpaceClass
 *  @li dSweepAndPruneSpaceClass
 *  @li dQuadTreeSpaceClass
 *  @li dFirstUserClass
 *  @li dLastUserClass
 *
 * The class id not defined by the user should be between
 * dFirstSpaceClass and dLastSpaceClass.
 *
 * User-defined class will return their own number.
 *
 * @param space the space to query
 * @returns The space class ID.
 * @ingroup collide
 */
ODE_API int dSpaceGetClass(dSpaceID space);

#ifdef __cplusplus
}
#endif

#endif
