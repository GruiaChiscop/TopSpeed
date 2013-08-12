/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifndef __COMMON_TRACER_H__
#define __COMMON_TRACER_H__

#include <Common/If/Common.h>
#include <Common/If/File.h>


class File;

class Tracer
{
public:
    ///@name Constructor and destructor
    //@{
    _common_ Tracer(const Char name[] = ".");
    _common_ virtual ~Tracer( );
    //@}
public:
    _common_ void bind(File*);

    _common_ void trace(const Char*, ...);
    
public:
    ///@name 'enable - disable' methods
    //@{
    _common_ void enable( )       { m_enabled = true;  }
    _common_ void disable( )      { m_enabled = false; }
    //@}

    ///@name 'get' methods
    //@{
    _common_ Boolean enabled( )   { return m_enabled;  } 
    //@}

private:
    Boolean          m_enabled;
    const Char*      m_name;
    File*            m_file;
};







#endif /* __COMMON_TRACER_H__ */
