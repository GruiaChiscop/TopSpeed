
/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/#ifndef __COMMON_FILE_H__
#define __COMMON_FILE_H__

#include <Common/If/Common.h>
#include <stdio.h>

class File
{
public:
    enum
    {
        read   = 0x01,
        write  = 0x02,
        append = 0x04,
        create = 0x08
    };

public:
    _common_ File(const Char filename[], UInt flags);
    _common_ virtual ~File( );

public:
    _common_ void rewind( );

public:
    _common_ void writeInt(Int i);
    _common_ Int  readInt( );

    _common_ void writeFloat(Float f);
    _common_ Float readFloat( );

    _common_ void writeChar(Char c);
    _common_ Char readChar( );

    _common_ void writeString(Char* string);

    _common_ Boolean readInt(Char* field, Int& val, Int def = 0);
    _common_ Boolean readFloat(Char* field, Float& val, Float def = 0.0f);
    _common_ Boolean readString(Char* field, Char* val, UInt size, Char* def = 0);
    _common_ Boolean readKeyInt(Char* field, Int& val, Int def = 0);
    _common_ Boolean writeKeyString(Char* field, Char* val);
    _common_ Boolean writeKeyInt(Char* field, Int val);
    _common_ Boolean readVarString(Char* field, Char* val, UInt sizeVal, Char* var, UInt sizeVar, UInt count, Char* def = 0);


public:
    ///@name 'get' methods
    //@{
    _common_ FILE*  getStream( )    { return m_file;   }
    _common_ Boolean opened( )      { return m_opened; }
    //@}

private:
    FILE*        m_file;
    Boolean      m_opened;

};


#endif __COMMON_FILE_H__
