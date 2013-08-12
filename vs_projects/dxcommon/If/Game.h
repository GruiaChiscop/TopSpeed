#ifndef __DXCOMMON_GAME_H__
#define __DXCOMMON_GAME_H__

#include <DxCommon/If/Internal.h>
#include <DxCommon/If/Input.h>
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
class Game
{
public:
    _dxcommon_ Game( );
    _dxcommon_ virtual ~Game( );

public:
    _dxcommon_ virtual Int   create(Window::Instance, Int icon);

    _dxcommon_ virtual Int   initialize( )                         { return dxSuccess; }
    _dxcommon_ virtual void  finalize( )                           {                   }

    _dxcommon_ virtual Int   run(/* Int keyboardAccelerator */);

    _dxcommon_ virtual Int   prepareNextFrame(/* Float elapsed */)       { return dxSuccess; }
    _dxcommon_ virtual Int   render(/* Float elapsed */)                 { return dxSuccess; }

    _dxcommon_ virtual Int   displayMode(Int width, Int height, Boolean fullscreen, 
                                         Boolean bit32 = false, Boolean antialiasing = false);

public:
    _dxcommon_ virtual void  showFPS(Boolean b)                    { m_showFPS = b; }

    _dxcommon_ const   Int   backgroundColor( ) const              { return m_backgroundColor; }
    _dxcommon_         void  backgroundColor(Int c)                { m_backgroundColor = c; }

    _dxcommon_         void  enableFog(UInt color, Float near, Float far /*, Float density */);
    _dxcommon_         void  disableFog( );

    _dxcommon_         void  enableLinearFilter( )                 { m_textureFilter = D3DTEXF_LINEAR; }
    _dxcommon_         void  enablePointFilter( )                  { m_textureFilter = D3DTEXF_POINT;  }

    _dxcommon_ LPDIRECT3DDEVICE8 d3dDevice( )                      { return m_d3dDevice; }

public:
    _dxcommon_ class Control3D
    {
    public: 
        _dxcommon_ Control3D(Game* game, LPDIRECT3DDEVICE8 device, Int screenWidth, Int screenHeight);
        _dxcommon_ virtual ~Control3D( );

    public:
        _dxcommon_ void view(Float fromx, Float fromy, Float fromz,
                             Float tox = 0, Float toy = 0, Float toz = 0);
        _dxcommon_ void view(Vector3 from, Vector3 to);
        _dxcommon_ void projection(Float Near, Float Far);
        _dxcommon_ void projection(Float Near, Float Far, Float angle);
        _dxcommon_ void material(UInt color, Float alpha = 1.0f);
        _dxcommon_ void ambient(UInt color);
        _dxcommon_ void alpha(Boolean enable);
        _dxcommon_ void antialiasing(Boolean enable);
        _dxcommon_ void position(LPDIRECT3DDEVICE8 d3dDevice, Float x, Float y, Float z);
        _dxcommon_ void position(LPDIRECT3DDEVICE8 d3dDevice, Vector3 pos);
        _dxcommon_ void yawPitchRoll(LPDIRECT3DDEVICE8 d3dDevice, Float yaw, Float pitch, Float roll);
        _dxcommon_ void yawPitchRoll(LPDIRECT3DDEVICE8 d3dDevice, Vector3 ypr);
        _dxcommon_ void rotateAxis(LPDIRECT3DDEVICE8 d3dDevice, DirectX::Vector3 axis, Float angle);
        _dxcommon_ void scale(LPDIRECT3DDEVICE8 d3dDevice, Float scale);
        _dxcommon_ void scale(LPDIRECT3DDEVICE8 d3dDevice, Float scaleX, Float scaleY, Float scaleZ);
        _dxcommon_ void reset(LPDIRECT3DDEVICE8 d3dDevice);

        _dxcommon_ D3DXMATRIX& viewMatrix( )   { return m_viewMatrix; }
        _dxcommon_ Float       viewAngle( )    { return m_angle;      }

    protected:
        Game*                   m_game;
        LPDIRECT3DDEVICE8       m_d3dDevice;
        Int                     m_screenWidth;
        Int                     m_screenHeight;
        Float                   m_angle;
        D3DXMATRIX              m_viewMatrix;
    };

public:
    Control3D*      control3D( )           { return m_control3D;          }
    Input::State    inputState( )          { return m_input->state( );    }
    D3DCAPS8&       capabilities( )        { return m_d3dCapabilities;    }
    Boolean         multisampleCapable( )  { return m_multisampleCapable; }
    Boolean         bit32Capable( )        { return m_32bitCapable;       }
    D3DFORMAT       textureFormat( )       { return m_d3dTextureFormat;   }
    Int             screenWidth( )         { return m_screenWidth;        }
    Int             screenHeight( )        { return m_screenHeight;       }
    Window::Handle  mainWindow( )          { return m_mainWindow;         }

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
    D3DFORMAT               m_d3dFullscreenFormat32;      // Pixel format for fullscreen modes, 32 bit
    D3DFORMAT               m_d3dWindowedFormat;
    D3DFORMAT               m_d3dTextureFormat;           // Pixel format for textures
    D3DCAPS8                m_d3dCapabilities;
    Boolean                 m_multisampleCapable;
    Boolean                 m_32bitCapable;
    HWND                    m_windowHandle;
    UInt                    m_textureFilter;
    
    LPDIRECT3DDEVICE8       m_d3dDevice;           // Class to handle D3D device
    D3DPRESENT_PARAMETERS   m_d3dPresentParameters;

    D3DFont*                m_font;
    InputManager*           m_input;
    Timer                   m_timer;
    Huge                    m_elapsed;

    Boolean                 m_showFPS;
    Int                     m_backgroundColor;
};


} // namespace DirectX



#endif /* __DXCOMMON_GAME_H__ */
