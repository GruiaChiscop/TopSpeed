#include <DxCommon/If/Light.h>

namespace DirectX
{


Light::Light(Type type, Vector3 position, Vector3 direction)
{
    ZeroMemory(&m_light, sizeof(D3DLIGHT8));
    m_light.Type        = (_D3DLIGHTTYPE) type;
    m_light.Diffuse.r   = 1.0f;
    m_light.Diffuse.g   = 1.0f;
    m_light.Diffuse.b   = 1.0f;
    m_light.Ambient     = m_light.Diffuse;
    m_light.Specular    = m_light.Diffuse;
    D3DXVec3Normalize((D3DXVECTOR3*)&m_light.Direction, &D3DXVECTOR3(direction.x, direction.y, direction.z));
    m_light.Position.x   = position.x;
    m_light.Position.y   = position.y;
    m_light.Position.z   = position.z;
    m_light.Range        = 1000.0f;
}




Light::Light(Type type, Vector3 position, Vector3 direction, Int color)
{
    ZeroMemory(&m_light, sizeof(D3DLIGHT8));
    m_light.Type        = (_D3DLIGHTTYPE) type;
    m_light.Diffuse.r   = ((color & 0xff0000) >> 16) / 255.0f;
    m_light.Diffuse.g   = ((color & 0x00ff00) >> 8) / 255.0f;
    m_light.Diffuse.b   = (color & 0x0000ff) / 255.0f;
    D3DXVec3Normalize((D3DXVECTOR3*)&m_light.Direction, &D3DXVECTOR3(direction.x, direction.y, direction.z));
    m_light.Position.x   = position.x;
    m_light.Position.y   = position.y;
    m_light.Position.z   = position.z;
    m_light.Range        = 1000.0f;
}



Light::~Light( )
{


}


void Light::enable(LPDIRECT3DDEVICE8 d3dDevice, UInt number)
{
    d3dDevice->SetLight(number, &m_light);
	d3dDevice->LightEnable(number, TRUE);    
}


void Light::disable(LPDIRECT3DDEVICE8 d3dDevice, UInt number)
{
    d3dDevice->LightEnable(number, FALSE);    
}


} // namespace DirectX