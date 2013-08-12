/**
* DXCommon library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifndef __DXCOMMON_D3DFONT_H__
#define __DXCOMMON_D3DFONT_H__

#include <DxCommon/If/Internal.h>
#include <D3D8.h>


namespace DirectX
{

class D3DFont : public DeviceDependent
{
public:
    _dxcommon_ enum
    {
        normal      = 0x0000,
        bold        = 0x0001,
        italic      = 0x0002,
        zenable     = 0x0004
    };

    _dxcommon_ enum
    {
        left        = 0x0000,
        centered    = 0x0001,
        twosided    = 0x0002,
        filtered    = 0x0004
    };

public:
    _dxcommon_ D3DFont(Char* fontName, UInt size, UInt flags = 0);
    _dxcommon_ virtual ~D3DFont( );

public:
    ///@name Interface 'DeviceDependent' methods
    //@{
    _dxcommon_ virtual Int initDevices(LPDIRECT3DDEVICE8 d3dDevice);
    _dxcommon_ virtual Int restoreDevices(/* LPDIRECT3DDEVICE8 d3dDevice */);
    _dxcommon_ virtual Int invalidateDevices( );
    _dxcommon_ virtual Int deleteDevices( );
    //@}

public:
    _dxcommon_ virtual Int drawText(Float x, Float y, UInt color, Char* text, Int flags = 0);
    _dxcommon_ virtual Int draw3DText(Char* text, Int flags = 0);

    _dxcommon_ SIZE    size(Char* text);

private:
    Char                    m_fontName[80];
    UInt                    m_size;
    UInt                    m_flags;

    LPDIRECT3DDEVICE8       m_d3dDevice; // A D3DDevice used for rendering
    LPDIRECT3DTEXTURE8      m_texture;   // The d3d texture for this font
    LPDIRECT3DVERTEXBUFFER8 m_vertexBuffer;        // VertexBuffer for rendering text

    // Stateblocks for setting and restoring render states
    DWORD                   m_savedStateBlock;
    DWORD                   m_drawTextStateBlock;
    UInt                    m_textureWidth;
    UInt                    m_textureHeight;
    Float                   m_scale;
    Float                   m_textureCoords[128-32][4];



/*    
public:
    // 2D and 3D text drawing functions
    HRESULT DrawText( FLOAT x, FLOAT y, DWORD dwColor, 
                      TCHAR* strText, DWORD dwFlags=0L );
    HRESULT DrawTextScaled( FLOAT x, FLOAT y, FLOAT z, 
                            FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
                            TCHAR* strText, DWORD dwFlags=0L );
    HRESULT Render3DText( TCHAR* strText, DWORD dwFlags=0L );
    
    // Function to get extent of text
    HRESULT GetTextExtent( TCHAR* strText, SIZE* pSize );
*/

};



} // namespace DirectX


#endif /* __DXCOMMON_D3DFONT_H__ */
