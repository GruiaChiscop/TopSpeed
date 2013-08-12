#include <DxCommon/If/Common.h>
#include <tchar.h>


namespace DirectX
{

//-----------------------------------------------------------------------------
// Custom vertex types for rendering text
//-----------------------------------------------------------------------------
#define MAX_NUM_VERTICES 50*6

struct Vertex2DFont
{
    D3DXVECTOR4  p;
    UInt          color; 
    Float        tu, tv;
};

#define FVF_VERTEX2DFONT (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)


inline Vertex2DFont initVertex2DFont( const D3DXVECTOR4& p, D3DCOLOR color,
                                      Float tu, FLOAT tv )
{
    Vertex2DFont v;   v.p = p;   v.color = color;   v.tu = tu;   v.tv = tv;
    return v;
}


struct Vertex3DFont
{
    D3DXVECTOR3  p;
    D3DXVECTOR3  n;
    Float        tu, tv;
};

#define FVF_VERTEX3DFONT (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


inline Vertex3DFont initVertex3DFont( const D3DXVECTOR3& p, const D3DXVECTOR3& n,
                                      Float tu, FLOAT tv )
{
    Vertex3DFont v;   v.p = p;   v.n = n;   v.tu = tu;   v.tv = tv;
    return v;
}




//-----------------------------------------------------------------------------
// Name: CD3DFont()
// Desc: Font class constructor
//-----------------------------------------------------------------------------
D3DFont::D3DFont(Char* fontName, UInt size, UInt flags )
{
    DXCOMMON("(+) D3DFont (name = %s, size = %d, flags = %d).", fontName, size, flags);
    _tcscpy(m_fontName, fontName );
    m_size                 = size;
    m_flags                = flags;

    m_d3dDevice            = 0;
    m_texture              = 0;
    m_vertexBuffer         = 0;

    m_savedStateBlock      = 0L;
    m_drawTextStateBlock   = 0L;
}




//-----------------------------------------------------------------------------
// Name: ~CD3DFont()
// Desc: Font class destructor
//-----------------------------------------------------------------------------
D3DFont::~D3DFont()
{
    DXCOMMON("(-) D3DFont (name = %s).", m_fontName);
    invalidateDevices();
    deleteDevices();
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initializes device-dependent objects, including the vertex buffer used
//       for rendering text and the texture map which stores the font image.
//-----------------------------------------------------------------------------
Int D3DFont::initDevices(LPDIRECT3DDEVICE8 d3dDevice)
{
    HRESULT hr;

    // Keep a local copy of the device
    m_d3dDevice = d3dDevice;

    // Establish the font and texture size
    m_scale  = 1.0f; // Draw fonts into texture without scaling

    // Large fonts need larger textures
    if (m_size > 40 )
        m_textureWidth = m_textureHeight = 1024;
    else if( m_size > 20 )
        m_textureWidth = m_textureHeight = 512;
    else
        m_textureWidth  = m_textureHeight = 256;

    // If requested texture is too big, use a smaller texture and smaller font,
    // and scale up when rendering.
    D3DCAPS8 d3dCaps;
    m_d3dDevice->GetDeviceCaps(&d3dCaps);

    if( m_textureWidth > d3dCaps.MaxTextureWidth )
    {
        m_scale        = (Float)d3dCaps.MaxTextureWidth / (Float)m_textureWidth;
        m_textureWidth = m_textureHeight = d3dCaps.MaxTextureWidth;
    }

    // Create a new texture for the font
    hr = m_d3dDevice->CreateTexture( m_textureWidth, m_textureHeight, 1,
                                     0, D3DFMT_A4R4G4B4,
                                     D3DPOOL_MANAGED, &m_texture );
    if (FAILED(hr))
    {
        DXCOMMON("(!) D3DFont::initDevices : failed to create texture.");
        return dxFailed;
    }
        
    // Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)m_textureWidth;
    bmi.bmiHeader.biHeight      = -(int)m_textureHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // Create a DC and a bitmap for the font
    HDC     hDC       = CreateCompatibleDC( NULL );
    HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );
    SetMapMode(hDC, MM_TEXT);

    // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
    // antialiased font, but this is not guaranteed.
    INT nHeight    = -MulDiv( m_size, 
        (INT)(GetDeviceCaps(hDC, LOGPIXELSY) * m_scale), 72 );
    DWORD dwBold   = (m_flags & bold)   ? FW_BOLD : FW_NORMAL;
    DWORD dwItalic = (m_flags & italic) ? TRUE    : FALSE;
    HFONT hFont    = CreateFont( nHeight, 0, 0, 0, dwBold, dwItalic,
                          FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                          VARIABLE_PITCH, m_fontName );
    if (hFont == 0)
    {
        DXCOMMON("(!) D3DFont::initDevices ; failed to create font (name = %s).", m_fontName);
        return dxFailed;
    }

    SelectObject(hDC, hbmBitmap);
    SelectObject(hDC, hFont);

    // Set text properties
    SetTextColor( hDC, RGB(255,255,255) );
    SetBkColor(   hDC, 0x00000000 );
    SetTextAlign( hDC, TA_TOP );

    // Loop through all printable character and output them to the bitmap..
    // Meanwhile, keep track of the corresponding tex coords for each character.
    DWORD x = 0;
    DWORD y = 0;
    TCHAR str[2] = _T("x");
    SIZE size;

    for( TCHAR c=32; c<127; c++ )
    {
        str[0] = c;
        GetTextExtentPoint32( hDC, str, 1, &size );

        if( (DWORD)(x+size.cx+1) > m_textureWidth )
        {
            x  = 0;
            y += size.cy+1;
        }

        ExtTextOut( hDC, x+0, y+0, ETO_OPAQUE, NULL, str, 1, NULL );

        m_textureCoords[c-32][0] = ((FLOAT)(x+0))/m_textureWidth;
        m_textureCoords[c-32][1] = ((FLOAT)(y+0))/m_textureHeight;
        m_textureCoords[c-32][2] = ((FLOAT)(x+0+size.cx))/m_textureWidth;
        m_textureCoords[c-32][3] = ((FLOAT)(y+0+size.cy))/m_textureHeight;

        x += size.cx+1;
    }

    // Lock the surface and write the alpha values for the set pixels
    D3DLOCKED_RECT d3dlr;
    m_texture->LockRect( 0, &d3dlr, 0, 0 );
    BYTE* pDstRow = (BYTE*)d3dlr.pBits;
    WORD* pDst16;
    BYTE bAlpha; // 4-bit measure of pixel intensity

    for( y=0; y < m_textureHeight; y++ )
    {
        pDst16 = (WORD*)pDstRow;
        for( x=0; x < m_textureWidth; x++ )
        {
            bAlpha = (BYTE)((pBitmapBits[m_textureWidth*y + x] & 0xff) >> 4);
            if (bAlpha > 0)
            {
                *pDst16++ = (bAlpha << 12) | 0x0fff;
            }
            else
            {
                *pDst16++ = 0x0000;
            }
        }
        pDstRow += d3dlr.Pitch;
    }

    // Done updating texture, so clean up used objects
    m_texture->UnlockRect(0);
    DeleteObject( hbmBitmap );
    DeleteDC( hDC );
    DeleteObject( hFont );

    return dxSuccess;
}



//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
Int D3DFont::restoreDevices(/* LPDIRECT3DDEVICE8 d3dDevice */)
{
    HRESULT hr;

    // Create vertex buffer for the letters
    if (FAILED(hr = m_d3dDevice->CreateVertexBuffer( MAX_NUM_VERTICES*sizeof(Vertex2DFont),
                                                      D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
                                                      D3DPOOL_DEFAULT, &m_vertexBuffer)))
    {
        DXCOMMON("(!) D3DFont::restoreDevices : failed to create VertexBuffer");
        return dxFailed;
    }

    // Create the state blocks for rendering text
    for (UInt which=0; which<2; ++which)
    {
        m_d3dDevice->BeginStateBlock();
        m_d3dDevice->SetTexture( 0, m_texture );

        if (zenable & m_flags )
            m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        else
            m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

        m_d3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        m_d3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
        m_d3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
        m_d3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
        m_d3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
        m_d3dDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
        m_d3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
        m_d3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
        m_d3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
        m_d3dDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
        m_d3dDevice->SetRenderState( D3DRS_EDGEANTIALIAS,    FALSE );
        m_d3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
        m_d3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
        m_d3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
        m_d3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        m_d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        m_d3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

        if( which==0 )
            m_d3dDevice->EndStateBlock(&m_savedStateBlock);
        else
            m_d3dDevice->EndStateBlock(&m_drawTextStateBlock);
    }

    return dxSuccess;
}


//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
Int D3DFont::invalidateDevices()
{
    SAFE_RELEASE(m_vertexBuffer);

    // Delete the state blocks
    if (m_d3dDevice)
    {
        if (m_savedStateBlock)
            m_d3dDevice->DeleteStateBlock( m_savedStateBlock );
        if (m_drawTextStateBlock)
            m_d3dDevice->DeleteStateBlock( m_drawTextStateBlock );
    }

    m_savedStateBlock    = 0L;
    m_drawTextStateBlock = 0L;

    return dxSuccess;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
Int D3DFont::deleteDevices()
{
    SAFE_RELEASE(m_texture);
    m_d3dDevice = NULL;

    return dxSuccess;
}



Int D3DFont::drawText(Float x, Float y, UInt color, Char* text, Int flags)
{
    if (m_d3dDevice == 0)
    {
        DXCOMMON("(!) D3DFont::drawText : 3D device not initialized.");
        return dxFailed;
    }

    // Setup renderstate
    m_d3dDevice->CaptureStateBlock(m_savedStateBlock);
    m_d3dDevice->ApplyStateBlock(m_drawTextStateBlock);
    m_d3dDevice->SetVertexShader(FVF_VERTEX2DFONT);
    m_d3dDevice->SetPixelShader(0);
    m_d3dDevice->SetStreamSource(0, m_vertexBuffer, sizeof(Vertex2DFont));

    // Set filter states
    if (flags & filtered)
    {
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    Float fStartX = x;

    // Fill vertex buffer
    Vertex2DFont* pVertices = 0;
    DWORD         dwNumTriangles = 0;
    m_vertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    while (*text)
    {
        TCHAR c = *text++;

        if (c == _T('\n'))
        {
            x = fStartX;
            y += (m_textureCoords[0][3]-m_textureCoords[0][1])*m_textureHeight;
        }
        if (c < _T(' '))
            continue;

        FLOAT tx1 = m_textureCoords[c-32][0];
        FLOAT ty1 = m_textureCoords[c-32][1];
        FLOAT tx2 = m_textureCoords[c-32][2];
        FLOAT ty2 = m_textureCoords[c-32][3];

        FLOAT w = (tx2-tx1) * m_textureWidth / m_scale;
        FLOAT h = (ty2-ty1) * m_textureHeight / m_scale;

        if( c != _T(' ') )
        {
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+0-0.5f,y+h-0.5f,0.9f,1.0f), color, tx1, ty2 );
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+0-0.5f,y+0-0.5f,0.9f,1.0f), color, tx1, ty1 );
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+w-0.5f,y+h-0.5f,0.9f,1.0f), color, tx2, ty2 );
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+w-0.5f,y+0-0.5f,0.9f,1.0f), color, tx2, ty1 );
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+w-0.5f,y+h-0.5f,0.9f,1.0f), color, tx2, ty2 );
            *pVertices++ = initVertex2DFont( D3DXVECTOR4(x+0-0.5f,y+0-0.5f,0.9f,1.0f), color, tx1, ty1 );
            dwNumTriangles += 2;

            if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
            {
                // Unlock, render, and relock the vertex buffer
                m_vertexBuffer->Unlock();
                m_d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                pVertices = NULL;
                m_vertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        x += w;
    }

    // Unlock and render the vertex buffer
    m_vertexBuffer->Unlock();
    if( dwNumTriangles > 0 )
        m_d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_d3dDevice->ApplyStateBlock( m_savedStateBlock );

    return dxSuccess;
}


Int D3DFont::draw3DText(Char* text, Int flags)
{
    if (m_d3dDevice == 0)
    {
        DXCOMMON("(!) D3DFont::draw3DText : 3D device not initialized.");
        return dxFailed;
    }

    // Setup renderstate
    m_d3dDevice->CaptureStateBlock( m_savedStateBlock );
    m_d3dDevice->ApplyStateBlock( m_drawTextStateBlock );
    m_d3dDevice->SetVertexShader( FVF_VERTEX3DFONT );
    m_d3dDevice->SetPixelShader( NULL );
    m_d3dDevice->SetStreamSource( 0, m_vertexBuffer, sizeof(Vertex3DFont));

    // Set filter states
    if (flags & filtered )
    {
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    // Position for each text element
    FLOAT x = 0.0f;
    FLOAT y = 0.0f;

    // Center the text block at the origin
    if (flags & centered)
    {
        SIZE sz = size(text);
        x = -(((FLOAT)sz.cx)/10.0f)/2.0f;
        y = -(((FLOAT)sz.cy)/10.0f)/2.0f;
    }

    // Turn off culling for two-sided text
    if (flags & twosided)
        m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    FLOAT fStartX = x;
    TCHAR c;

    // Fill vertex buffer
    Vertex3DFont* pVertices;
    // DWORD         dwVertex       = 0L;
    DWORD         dwNumTriangles = 0L;
    m_vertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    while( c = *text++ )
    {
        if( c == '\n' )
        {
            x = fStartX;
            y -= (m_textureCoords[0][3]-m_textureCoords[0][1])*m_textureHeight/10.0f;
        }
        if( c < 32 )
            continue;

        FLOAT tx1 = m_textureCoords[c-32][0];
        FLOAT ty1 = m_textureCoords[c-32][1];
        FLOAT tx2 = m_textureCoords[c-32][2];
        FLOAT ty2 = m_textureCoords[c-32][3];

        FLOAT w = (tx2-tx1) * m_textureWidth  / ( 10.0f * m_scale );
        FLOAT h = (ty2-ty1) * m_textureHeight / ( 10.0f * m_scale );

        if( c != _T(' ') )
        {
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+0,y+0,0), D3DXVECTOR3(0,0,-1), tx1, ty2 );
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+w,y+h,0), D3DXVECTOR3(0,0,-1), tx2, ty1 );
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = initVertex3DFont( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            dwNumTriangles += 2;

            if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
            {
                // Unlock, render, and relock the vertex buffer
                m_vertexBuffer->Unlock();
                m_d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                m_vertexBuffer->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        x += w;
    }

    // Unlock and render the vertex buffer
    m_vertexBuffer->Unlock();
    if( dwNumTriangles > 0 )
        m_d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_d3dDevice->ApplyStateBlock( m_savedStateBlock );

    return dxSuccess;
}



SIZE D3DFont::size(Char* text)
{
    SIZE result = { 0, 0 };
    //if( NULL==strText || NULL==pSize )
    //    return E_FAIL;

    FLOAT fRowWidth  = 0.0f;
    FLOAT fRowHeight = (m_textureCoords[0][3]-m_textureCoords[0][1])*m_textureHeight;
    FLOAT fWidth     = 0.0f;
    FLOAT fHeight    = fRowHeight;

    while( *text )
    {
        TCHAR c = *text++;

        if( c == _T('\n') )
        {
            fRowWidth = 0.0f;
            fHeight  += fRowHeight;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_textureCoords[c-32][0];
        FLOAT tx2 = m_textureCoords[c-32][2];

        fRowWidth += (tx2-tx1)*m_textureWidth;

        if( fRowWidth > fWidth )
            fWidth = fRowWidth;
    }

    result.cx = (int)fWidth;
    result.cy = (int)fHeight;

    return result;
}



/*
//-----------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: Get the dimensions of a text string
//-----------------------------------------------------------------------------
HRESULT CD3DFont::GetTextExtent( TCHAR* strText, SIZE* pSize )
{
    if( NULL==strText || NULL==pSize )
        return E_FAIL;

    FLOAT fRowWidth  = 0.0f;
    FLOAT fRowHeight = (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight;
    FLOAT fWidth     = 0.0f;
    FLOAT fHeight    = fRowHeight;

    while( *strText )
    {
        TCHAR c = *strText++;

        if( c == _T('\n') )
        {
            fRowWidth = 0.0f;
            fHeight  += fRowHeight;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT tx2 = m_fTexCoords[c-32][2];

        fRowWidth += (tx2-tx1)*m_dwTexWidth;

        if( fRowWidth > fWidth )
            fWidth = fRowWidth;
    }

    pSize->cx = (int)fWidth;
    pSize->cy = (int)fHeight;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawTextScaled()
// Desc: Draws scaled 2D text.  Note that x and y are in viewport coordinates
//       (ranging from -1 to +1).  fXScale and fYScale are the size fraction 
//       relative to the entire viewport.  For example, a fXScale of 0.25 is
//       1/8th of the screen width.  This allows you to output text at a fixed
//       fraction of the viewport, even if the screen or window size changes.
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DrawTextScaled( FLOAT x, FLOAT y, FLOAT z,
                                  FLOAT fXScale, FLOAT fYScale, DWORD dwColor,
                                  TCHAR* strText, DWORD dwFlags )
{
    if( m_pd3dDevice == NULL )
        return E_FAIL;

    // Set up renderstate
    m_pd3dDevice->CaptureStateBlock( m_dwSavedStateBlock );
    m_pd3dDevice->ApplyStateBlock( m_dwDrawTextStateBlock );
    m_pd3dDevice->SetVertexShader( D3DFVF_FONT2DVERTEX );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(FONT2DVERTEX) );

    // Set filter states
    if( dwFlags & D3DFONT_FILTERED )
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    D3DVIEWPORT8 vp;
    m_pd3dDevice->GetViewport( &vp );
    FLOAT sx  = (x+1.0f)*vp.Width/2;
    FLOAT sy  = (y+1.0f)*vp.Height/2;
    FLOAT sz  = z;
    FLOAT rhw = 1.0f;
    FLOAT fStartX = sx;

    FLOAT fLineHeight = ( m_fTexCoords[0][3] - m_fTexCoords[0][1] ) * m_dwTexHeight;

    // Fill vertex buffer
    FONT2DVERTEX* pVertices;
    DWORD         dwNumTriangles = 0L;
    m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    while( *strText )
    {
        TCHAR c = *strText++;

        if( c == _T('\n') )
        {
            sx  = fStartX;
            sy += fYScale*vp.Height;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT ty1 = m_fTexCoords[c-32][1];
        FLOAT tx2 = m_fTexCoords[c-32][2];
        FLOAT ty2 = m_fTexCoords[c-32][3];

        FLOAT w = (tx2-tx1)*m_dwTexWidth;
        FLOAT h = (ty2-ty1)*m_dwTexHeight;

        w *= (fXScale*vp.Width)/fLineHeight;
        h *= (fYScale*vp.Height)/fLineHeight;

        if( c != _T(' ') )
        {
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx1, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,sz,rhw), dwColor, tx1, ty1 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx2, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+0-0.5f,sz,rhw), dwColor, tx2, ty1 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx2, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,sz,rhw), dwColor, tx1, ty1 );
            dwNumTriangles += 2;

            if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
            {
                // Unlock, render, and relock the vertex buffer
                m_pVB->Unlock();
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        sx += w;
    }

    // Unlock and render the vertex buffer
    m_pVB->Unlock();
    if( dwNumTriangles > 0 )
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_pd3dDevice->ApplyStateBlock( m_dwSavedStateBlock );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws 2D text
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DrawText( FLOAT sx, FLOAT sy, DWORD dwColor,
                            TCHAR* strText, DWORD dwFlags )
{
    if( m_pd3dDevice == NULL )
        return E_FAIL;

    // Setup renderstate
    m_pd3dDevice->CaptureStateBlock( m_dwSavedStateBlock );
    m_pd3dDevice->ApplyStateBlock( m_dwDrawTextStateBlock );
    m_pd3dDevice->SetVertexShader( D3DFVF_FONT2DVERTEX );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(FONT2DVERTEX) );

    // Set filter states
    if( dwFlags & D3DFONT_FILTERED )
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    FLOAT fStartX = sx;

    // Fill vertex buffer
    FONT2DVERTEX* pVertices = NULL;
    DWORD         dwNumTriangles = 0;
    m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    while( *strText )
    {
        TCHAR c = *strText++;

        if( c == _T('\n') )
        {
            sx = fStartX;
            sy += (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight;
        }
        if( c < _T(' ') )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT ty1 = m_fTexCoords[c-32][1];
        FLOAT tx2 = m_fTexCoords[c-32][2];
        FLOAT ty2 = m_fTexCoords[c-32][3];

        FLOAT w = (tx2-tx1) *  m_dwTexWidth / m_fTextScale;
        FLOAT h = (ty2-ty1) * m_dwTexHeight / m_fTextScale;

        if( c != _T(' ') )
        {
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx1, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,0.9f,1.0f), dwColor, tx1, ty1 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx2, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+0-0.5f,0.9f,1.0f), dwColor, tx2, ty1 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx2, ty2 );
            *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,0.9f,1.0f), dwColor, tx1, ty1 );
            dwNumTriangles += 2;

            if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
            {
                // Unlock, render, and relock the vertex buffer
                m_pVB->Unlock();
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                pVertices = NULL;
                m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        sx += w;
    }

    // Unlock and render the vertex buffer
    m_pVB->Unlock();
    if( dwNumTriangles > 0 )
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_pd3dDevice->ApplyStateBlock( m_dwSavedStateBlock );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render3DText()
// Desc: Renders 3D text
//-----------------------------------------------------------------------------
HRESULT CD3DFont::Render3DText( TCHAR* strText, DWORD dwFlags )
{
    if( m_pd3dDevice == NULL )
        return E_FAIL;

    // Setup renderstate
    m_pd3dDevice->CaptureStateBlock( m_dwSavedStateBlock );
    m_pd3dDevice->ApplyStateBlock( m_dwDrawTextStateBlock );
    m_pd3dDevice->SetVertexShader( D3DFVF_FONT3DVERTEX );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(FONT3DVERTEX) );

    // Set filter states
    if( dwFlags & D3DFONT_FILTERED )
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    // Position for each text element
    FLOAT x = 0.0f;
    FLOAT y = 0.0f;

    // Center the text block at the origin
    if( dwFlags & D3DFONT_CENTERED )
    {
        SIZE sz;
        GetTextExtent( strText, &sz );
        x = -(((FLOAT)sz.cx)/10.0f)/2.0f;
        y = -(((FLOAT)sz.cy)/10.0f)/2.0f;
    }

    // Turn off culling for two-sided text
    if( dwFlags & D3DFONT_TWOSIDED )
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    FLOAT fStartX = x;
    TCHAR c;

    // Fill vertex buffer
    FONT3DVERTEX* pVertices;
    DWORD         dwVertex       = 0L;
    DWORD         dwNumTriangles = 0L;
    m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    while( c = *strText++ )
    {
        if( c == '\n' )
        {
            x = fStartX;
            y -= (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight/10.0f;
        }
        if( c < 32 )
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT ty1 = m_fTexCoords[c-32][1];
        FLOAT tx2 = m_fTexCoords[c-32][2];
        FLOAT ty2 = m_fTexCoords[c-32][3];

        FLOAT w = (tx2-tx1) * m_dwTexWidth  / ( 10.0f * m_fTextScale );
        FLOAT h = (ty2-ty1) * m_dwTexHeight / ( 10.0f * m_fTextScale );

        if( c != _T(' ') )
        {
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0,y+0,0), D3DXVECTOR3(0,0,-1), tx1, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w,y+h,0), D3DXVECTOR3(0,0,-1), tx2, ty1 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            dwNumTriangles += 2;

            if( dwNumTriangles*3 > (MAX_NUM_VERTICES-6) )
            {
                // Unlock, render, and relock the vertex buffer
                m_pVB->Unlock();
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        x += w;
    }

    // Unlock and render the vertex buffer
    m_pVB->Unlock();
    if( dwNumTriangles > 0 )
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_pd3dDevice->ApplyStateBlock( m_dwSavedStateBlock );

    return S_OK;
}




*/


} // namespace DirectX