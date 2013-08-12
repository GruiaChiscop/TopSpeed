/**
* DXCommon library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifndef __DXCOMMON_GAMEMENU_H__
#define __DXCOMMON_GAMEMENU_H__

#include <DxCommon/If/Internal.h>

#ifndef DXCOMMON_MAX_MENU_ITEMS
#define DXCOMMON_MAX_MENU_ITEMS  10
#endif

#ifndef DXCOMMON_MAX_MENU_ITEMSIZE
#define DXCOMMON_MAX_MENU_ITEMSIZE  64

namespace DirectX
{

class GameMenuItem
{
public:
    _dxcommon_ GameMenuItem(const Char text[]);
    _dxcommon_ virtual ~GameMenuItem( );

public:
    _dxcommon_ virtual void renderSelected(Float elapsed, Input::State);

private:
    Char            m_text[DXCOMMON_MAX_MENU_ITEMSIZE];
};


class GameMenuListItem
{
public:
    _dxcommon_ GameMenuListItem(const Char text[]);
    _dxcommon_ virtual ~GameMenuListItem( );

public:
    _dxcommon_ virtual void renderSelected(Float elapsed, Input::State);

public:
    _dxcommon_ GameMenuListItem*   next;
    _dxcommon_ GameMenuListItem*   previous;

private:
    Char            m_text[DXCOMMON_MAX_MENU_ITEMSIZE];
};


class GameMenuList : public GameMenuItem
{
public:
    _dxcommon_ GameMenuList(const Char Text[]);
    _dxcommon_ virutal ~GameMenuList( );

public:
    _dxcommon_ virtual void renderSelected(Float elapsed, Input::State);

public:
    TList<GameMenuListItem*>   m_sublist;
}


class GameMenu
{
public:
    _dxcommon_ GameMenu(D3DFont& font, Input& input);
    _dxcommon_ virtual ~GameMenu( );

public:
    _dxcommon_ void render(Float elapsed);
    _dxcommon_ void renderAt(UInt x, UInt y, Float elapsed);

private:
    GameMenuItem*    m_items[DXCOMMON_MAX_MENU_ITEMS];
    D3DFont         &m_font;
    Input           &m_input;
};


}; // namespace DirectX


#endif /* __DXCOMMON_GAMEMENU_H__ */

