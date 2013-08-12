#ifndef __DXCOMMON_APPLICATION_H__
#define __DXCOMMON_APPLICATION_H__

#include <DxCommon/If/Internal.h>
#include <DxCommon/If/D3DFont.h>
#include <DxCommon/If/Timer.h>
#include <D3DX8.h>

namespace DirectX
{

/*************************************************************************************
 *@
 *@
 *
 *************************************************************************************/
class Application
{
public:
    _dxcommon_ Application( );
    _dxcommon_ virtual ~Application( );

public:
    _dxcommon_ virtual Int   create(Window::Instance, Int icon);

    _dxcommon_ virtual Int   initialize( )                         { return dxSuccess; }
    _dxcommon_ virtual void  finalize( )                           {                   }

    _dxcommon_ virtual Int   run(/* Int keyboardAccelerator */);

    _dxcommon_ virtual Int   prepareNextFrame(/* Float elapsed */)       { return dxSuccess; }
    _dxcommon_ virtual Int   render(/* Float elapsed */)                 { return dxSuccess; }

    _dxcommon_ virtual Int   displayMode(Int width, Int height, Boolean fullscreen);

public:
    _dxcommon_ virtual void  showFPS(Boolean b)                    { m_showFPS = b; }

    _dxcommon_ const   Int   backgroundColor( ) const              { return m_backgroundColor; }
    _dxcommon_         void  backgroundColor(Int c)                { m_backgroundColor = c; }

public:
    _dxcommon_ class Control3D
    {
    public: 
        _dxcommon_ Control3D(LPDIRECT3DDEVICE8 device, Int screenWidth, Int screenHeight);
        _dxcommon_ virtual ~Control3D( );

    public:
        _dxcommon_ void view(Float fromx, Float fromy, Float fromz,
                             Float tox = 0, Float toy = 0, Float toz = 0);
        _dxcommon_ void view(Vector3 from, Vector3 to);
        _dxcommon_ void projection(Float Near, Float Far);
        _dxcommon_ void material(UInt color, Float alpha = 1.0f);
        _dxcommon_ void ambient(UInt color);

    protected:
        LPDIRECT3DDEVICE8       m_d3dDevice;
        Int                     m_screenWidth;
        Int                     m_screenHeight;
    };

public:
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    _dxcommon_ virtual Int   initializeDisplay( );



protected:
    Control3D*              m_control3D;
    Window::Handle          m_mainWindow;
    Boolean                 m_active;
    Boolean                 m_fullscreen;
    D3DDISPLAYMODE          m_desktopMode;
    Int                     m_screenWidth;
    Int                     m_screenHeight;
    D3DFORMAT               m_d3dFullscreenFormat;        // Pixel format for fullscreen modes
    D3DFORMAT               m_d3dWindowedFormat;
    D3DFORMAT               m_d3dTextureFormat;           // Pixel format for textures
    
    LPDIRECT3DDEVICE8       m_d3dDevice;           // Class to handle D3D device
    D3DPRESENT_PARAMETERS   m_d3dPresentParameters;

    D3DFont*                m_font;
    Timer                   m_timer;
    Huge                    m_elapsed;

    Boolean                 m_showFPS;
    Int                     m_backgroundColor;
};


} // namespace DirectX



#endif /* __DXCOMMON_APPLICATION_H__ */
