/**
* DXCommon library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#include <DxCommon/If/Particle.h>
#include <Common/If/Algorithm.h>
#include <DxCommon/If/Utilities.h>

#include <D3dx8math.h>

#define PARTICLE_FLUSH       512
#define PARTICLE_DISCARD    2048



namespace DirectX
{


struct POINTVERTEX
{
    D3DXVECTOR3 v;
    D3DCOLOR    color;
};

#define D3DFVF_POINTVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
ParticleSystem::ParticleSystem(LPDIRECT3DDEVICE8 d3dDevice, UInt nparticles,
                               D3DFORMAT textureFormat, Char* textureFile, Game* game) :
    m_d3dDevice(d3dDevice),
    m_game(game)
{
    DXCOMMON("(+) ParticleSystem.");
    m_base          = PARTICLE_DISCARD;
    m_flush         = PARTICLE_FLUSH;
	m_discard       = PARTICLE_DISCARD;

    m_nParticles    = 0;
    m_nMaxParticles = 10;

    m_particles     = 0;
    m_particlesFree = 0;
	m_vertexBuffer  = 0;

    // Create the particlepool
    for (UInt i = 0; i < nparticles; ++i)
    {
        Particle* temp = m_particlesFree;
        m_particlesFree = new Particle;
        m_particlesFree->next = temp;
        if (temp)
            m_particlesFree->next->prev = m_particlesFree;
    }

    if (createTexture(d3dDevice, textureFormat, &m_texture, textureFile) != dxSuccess)
    {
        DXCOMMON("(!) ParticleSystem : failed to create texture %s", textureFile);
    }
}



/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
ParticleSystem::~ParticleSystem()
{
    DXCOMMON("(-) ParticleSystem.");
	
    invalidateDevices();

    while (m_particles)
    {
        Particle* particle = m_particles;
        m_particles = particle->next;
        delete particle;
    }

    while (m_particlesFree)
    {
        Particle *particle = m_particlesFree;
        m_particlesFree = particle->next;
        delete particle;
    }
}


/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
void
ParticleSystem::emitFadingSingle(Vector3 position, UInt color, Float sceneTime)
{
    if (m_particlesFree)
    {
        // Get a free particle
        Particle* particle = m_particlesFree;
        m_particlesFree = particle->next;
        if (m_particlesFree)
        {
            particle->next->prev = 0;
            particle->next = m_particles;
        }
        particle->prev = 0;
        if (m_particles)
            m_particles->prev = particle;
        m_particles = particle;
        
        // Adjust settings for particle
        particle->position        = position;
        particle->velocity        = Vector3(0, 0, 0);

        particle->initPosition    = position;
        particle->initVelocity    = Vector3(0, 0, 0);
        particle->initTime        = sceneTime;

        particle->color           = color;
        particle->fadedColor      = (UInt)DirectX::Color::black;
        particle->fadeProgression = 1.0f;

        particle->type            = Particle::fadingSingle;
    }
    else
        DXCOMMON("(!) ParticleSystem::emitFadingSingle : no particles left!");
}


/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
Int ParticleSystem::restoreDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    // Create a vertex buffer for the particle system.  The size of this buffer
    // does not relate to the number of particles that exist.  Rather, the
    // buffer is used as a communication channel with the device.. we fill in 
    // a bit, and tell the device to draw.  While the device is drawing, we
    // fill in the next bit using NOOVERWRITE.  We continue doing this until 
    // we run out of vertex buffer space, and are forced to DISCARD the buffer
    // and start over at the beginning.

    if (FAILED(d3dDevice->CreateVertexBuffer(m_discard * sizeof(POINTVERTEX), 
                                             D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
		                                     D3DFVF_POINTVERTEX, D3DPOOL_DEFAULT, &m_vertexBuffer)))
	{
        DXCOMMON("(!) ParticleSystem::restoreDevices : failed to create vertexbuffer.");
        return dxFailed;
	}

    return dxSuccess;
}




/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
Int ParticleSystem::invalidateDevices()
{
    SAFE_RELEASE(m_vertexBuffer);

    return dxSuccess;
}







/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
void
ParticleSystem::prepareNextFrame(Float sceneTime, Float elapsed) 
{
    Particle* particle = m_particles;
    while (particle)
    {
        Particle* nextParticle = particle->next;
        if ((sceneTime - particle->initTime) < 1.0f)
        {
            particle->fadeProgression = sceneTime - particle->initTime + 0.01f;
        }
        else
        {
        
            particle->fadeProgression -= elapsed*0.3f;
        }
        if (particle->fadeProgression <= 0.0f)
        {
            // remove particle
            if (particle->prev)
                particle->prev->next = particle->next;
            if (particle->next)
                particle->next->prev = particle->prev;
            if (m_particlesFree)
                m_particlesFree->prev = particle;
            if (m_particles == particle)
                m_particles = particle->next;
            particle->next = m_particlesFree;
            m_particlesFree = particle;
        }

        particle = nextParticle;
    }
}



/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
Int ParticleSystem::render(/* Float sceneTime, Float elapsed */)
{
    /*
    m_d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_d3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    */
    //m_d3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
    //m_d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    m_d3dDevice->SetRenderState( D3DRS_LIGHTING,  FALSE );
    //m_d3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
    //m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

    m_d3dDevice->SetTexture(0, m_texture);

    m_d3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    m_d3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
    m_d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    // Set the render states for using point sprites
    m_d3dDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
    m_d3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE,  TRUE);
    if (m_game)
    {
        Float angle = m_game->control3D()->viewAngle();
        if (angle > 0.0f)
            m_d3dDevice->SetRenderState(D3DRS_POINTSIZE,     floatToDWORD(0.10f/angle));
        else
            m_d3dDevice->SetRenderState(D3DRS_POINTSIZE,     floatToDWORD(0.10f));
    }
    else
        m_d3dDevice->SetRenderState(D3DRS_POINTSIZE,     floatToDWORD(0.10f));
    m_d3dDevice->SetRenderState(D3DRS_POINTSIZE_MIN, floatToDWORD(0.01f));
    m_d3dDevice->SetRenderState(D3DRS_POINTSCALE_A,  floatToDWORD(0.00f));
    m_d3dDevice->SetRenderState(D3DRS_POINTSCALE_B,  floatToDWORD(1.00f));
    m_d3dDevice->SetRenderState(D3DRS_POINTSCALE_C,  floatToDWORD(0.00f));

   
    // Set up the vertex buffer to be rendered
    m_d3dDevice->SetStreamSource(0, m_vertexBuffer, sizeof(POINTVERTEX));
    m_d3dDevice->SetVertexShader(D3DFVF_POINTVERTEX);

    Particle*    particle = m_particles;
    POINTVERTEX* vertices;
    UInt         nParticlesToRender = 0;

	// Lock the vertex buffer.  We fill the vertex buffer in small
	// chunks, using D3DLOCK_NOOVERWRITE.  When we are done filling
	// each chunk, we call DrawPrim, and lock the next chunk.  When
	// we run out of space in the vertex buffer, we start over at
	// the beginning, using D3DLOCK_DISCARD.

    m_base += m_flush;

	if (m_base >= m_discard)
		m_base = 0;

	if (FAILED(m_vertexBuffer->Lock(m_base * sizeof(POINTVERTEX), m_flush * sizeof(POINTVERTEX),
		                            (BYTE **) &vertices, m_base ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
    {
        DXCOMMON("ParticleSystem::render : failed to lock vertexbuffer.");
		return dxFailed;
    }

    // Render each particle
    while (particle)
    {
        Vector3     position(particle->position);
        Vector3     velocity(particle->velocity);
        Float       lengthSq = velocity.lengthSquare( );
        UInt        nSteps;

        if      (lengthSq <  1.00f)  nSteps = 1;
        else if (lengthSq <  4.00f)  nSteps = 2;
        else if (lengthSq <  9.00f)  nSteps = 3;
        else if (lengthSq < 12.25f)  nSteps = 4;
        else if (lengthSq < 16.00f)  nSteps = 5;
        else if (lengthSq < 20.25f)  nSteps = 6;
        else                         nSteps = 7;

        velocity *= -0.04f / (Float) nSteps;

        D3DXCOLOR     diffuseColor;
        //D3DXColorLerp(&diffuseColor, (D3DXCOLOR*) &particle->fadedColor, (D3DXCOLOR*) &particle->color, particle->fadeProgression);
        D3DXColorLerp(&diffuseColor, (D3DXCOLOR*) (D3DXCOLOR*) &D3DXCOLOR( 0.1f,   0.0f,   0.0f,   1.0f ), (D3DXCOLOR*) &D3DXCOLOR( 0.0f,   0.5f,   0.5f,   1.0f ), particle->fadeProgression);
        UInt diffuse = (UInt) diffuseColor;

        // Render each particle a bunch of times to get a blurring effect
        for (UInt i = 0; i < nSteps; ++i) 
        {
            vertices->v     = D3DXVECTOR3(position.x, position.y, position.z);
            vertices->color = diffuse;
            ++vertices;

            if (++nParticlesToRender == m_flush)
            {
                // Done filling this chunk of the vertex buffer.  Lets unlock and
                // draw this portion so we can begin filling the next chunk.

                m_vertexBuffer->Unlock();

                if (FAILED(m_d3dDevice->DrawPrimitive(D3DPT_POINTLIST, m_base, nParticlesToRender)))
                {
                    DXCOMMON("(!) ParticleSystem::render : failed to draw primitives.");
					return dxFailed;
                }

                // Lock the next chunk of the vertex buffer.  If we are at the 
                // end of the vertex buffer, DISCARD the vertex buffer and start
                // at the beginning.  Otherwise, specify NOOVERWRITE, so we can
                // continue filling the VB while the previous chunk is drawing.
				m_base += m_flush;

				if (m_base >= m_discard)
					m_base = 0;

				if (FAILED(m_vertexBuffer->Lock(m_base * sizeof(POINTVERTEX), m_flush * sizeof(POINTVERTEX),
		                                        (BYTE **) &vertices, m_base ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
                {
                    DXCOMMON("(!) ParticleSystem::render : failed to lock vertexbuffer.");
					return dxFailed;
                }

                nParticlesToRender = 0;
            }

            position += velocity;
        }

        particle = particle->next;
    }

    // Unlock the vertex buffer
    m_vertexBuffer->Unlock();

    // Render any remaining particles
    if (nParticlesToRender)
    {
        if (FAILED(m_d3dDevice->DrawPrimitive(D3DPT_POINTLIST, m_base, nParticlesToRender)))
        {
            DXCOMMON("(!) ParticleSystem::render : failed to draw remaining particles.");
			return dxFailed;
        }
    }

    // Reset render states
    m_d3dDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    m_d3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE,  FALSE);

    m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    
    return dxSuccess;
}



}; // namespace DirectX



/*Point Sprites
Support for point sprites in Microsoft� Direct3D� for Microsoft DirectX� 8.1
enables the high-performance rendering of points (particle systems). Point
sprites are generalizations of generic points that enable arbitrary shapes
to be rendered as defined by textures.

Point Primitive Rendering Controls
Microsoft� Direct3D� for Microsoft DirectX� 8.1 supports additional
parameters to control the rendering of point sprites (point primitives).
These parameters enable points to be of variable size and have a full
texture map applied. The size of each point is determined by an
application-specified size combined with a distance-based function computed
by Direct3D. The application can specify point size either as per-vertex or
by setting D3DRS_POINTSIZE, which applies to points without a per-vertex
size. The point size is expressed in camera space units, with the exception
of when the application is passing post-transformed flexible vertex format
(FVF) vertices. In this case, the distance-based function is not applied and
the point size is expressed in units of pixels on the render target.

The texture coordinates computed and used when rendering points depends on
the setting of D3DRS_POINTSPRITEENABLE. When this value is set to TRUE, the
texture coordinates are set so that each point displays the full-texture. In
general, this is only useful when points are significantly larger than one
pixel. When D3DRS_POINTSPRITEENABLE is set to FALSE, each point's vertex
texture coordinate is used for the entire point.

Point Size Computations
Point size is determined by D3DRS_POINTSCALEENABLE. If this value is set to
FALSE, the application-specified point size is used as the screen-space
(post-transformed) size. Vertices that are passed to Microsoft� Direct3D� in
screen space do not have point sizes computed; the specified point size is
interpreted as screen-space size.

If D3DRS_POINTSCALEENABLE is TRUE, Direct3D computes the screen-space point
size according to the following formula. The application-specified point
size is expressed in camera space units.

Ss = Vh * Si * sqrt(1/(A + B*De + C*(De2)))

In this formula, the input point size, Si, is either per-vertex or the value
of the D3DRS_POINTSIZE render state. The point scale factors,
D3DRS_POINTSCALE_A, D3DRS_POINTSCALE_B, and D3DRS_POINTSCALE_C, are
represented by the points A, B, and C. The height of the viewport, Vh, is
the Height member of the D3DVIEWPORT8 structure representing the viewport.
De, the distance from the eye to the position (the eye at the origin), is
computed by taking the eye space position of the point (Xe, Ye, Ze) and
performing the following operation.

De = sqrt (Xe2 + Ye2 + Ze2)

The maximum point size, Pmax, is determined by taking the smaller of either
the MaxPointSize member of D3DCAPS8 or the D3DRS_POINTSIZE_MAX render state.
The minimum point size, Pmin, is determined by querying the value of
D3DRS_POINTSIZE_MIN. Thus the final screen-space point size, S, is
determined in the following manner.

If Ss > Pmax , then S = Pmax
if Ss < Pmin, then S = Pmin
Otherwise, S = Ss
Point Rendering
A screen-space point,

P = (X, Y, Z, W)

of screen-space size S is rasterized as a quadrilateral of the following
four vertices.

((X+S/2, Y+S/2, Z, W), (X+S/2, Y-S/2, Z, W), (X-S/2, Y-S/2, Z, W), (X-S/2,
Y+S/2, Z, W))

The vertex color attributes are duplicated at each vertex; thus each point
is always rendered with constant colors.

The assignment of texture indices is controlled by the
D3DRS_POINTSPRITEENABLE render state setting. If D3DRS_POINTSPRITEENABLE is
set to FALSE, then the vertex texture coordinates are duplicated at each
vertex. If D3DRS_POINTSPRITEENABLE is set to TRUE, then the texture
coordinates at the four vertices are set to the following values.

(0.F, 0.F), (0.F, 1.F), (1.F, 0.F), (1.F, 1.F)

This is shown in the following diagram.



When clipping is enabled, points are clipped in the following manner. If the
vertex exceeds the range of depth values-MinZ and MaxZ of the D3DVIEWPORT8
structure-into which a scene is to be rendered, the point exists outside of
the view frustum and is not rendered. If the point, taking into account the
point size, is completely outside the viewport in X and Y, then the point is
not rendered; the remaining points are rendered. It is possible for the
point position to be outside the viewport in X or Y and still be partially
visible.

Points may or may not be correctly clipped to user-defined clip planes. If
D3DPMISCCAPS_CLIPPLANESCALEDPOINTS is not set in the PrimitiveMiscCaps
member of D3DCAPS8, then points are clipped to user-defined clip planes
based only on the vertex position, ignoring the point size. In this case,
scaled points are fully rendered when the vertex position is inside the clip
planes, and discarded when the vertex position is outside a clip plane.
Applications can prevent potential artifacts by adding a border geometry to
clip planes that is as large as the maximum point size.

If the D3DPMISCCAPS_CLIPPLANESCALEDPOINTS bit is set, then the scaled points
are correctly clipped to user-defined clip planes.

Hardware vertex processing may or may not support point size. For example,
if a device is created with D3DCREATE_HARDWARE_VERTEXPROCESSING on a HAL
Device(D3DDEVTYPE_HAL) that has the MaxPointSize member of its D3DCAPS8
structure set to 1.0 or 0.0, then all points are a single pixel. To render
pixel point sprites less than 1.0, you must use either flexible vertex
format (FVF) TL (transformed and lit) vertices or software vertex processing
(D3DCREATE_SOFTWARE_VERTEXPROCESSING), in which case the Microsoft�
Direct3D� run time emulates the point sprite rendering. For details on FVF
TL vertices, see Transformed and Lit Vertex Functionality.

A hardware device that does vertex processing and supports point
sprites-MaxPointSize set to greater than 1.0f-is required to perform the
size computation for nontransformed sprites and is required to properly set
the per-vertex or D3DRS_POINTSIZE for TL vertices.
*/
