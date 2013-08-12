/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifndef __COMMON_ALGORITHM_H__
#define __COMMON_ALGORITHM_H__

template <class Type>
inline const Type& minimum(const Type& a, const Type& b)
{
    if (a < b)
        return a;
    else
        return b;
}


template <class Type>
inline const Type& maximum(const Type& a, const Type& b)
{
    if (a > b)
        return a;
    else
        return b;
}



template <class Type>
inline const Type absval(const Type& a)
{
    if (a < 0)
        return -a;
    else
        return a;
}

template <class Type>
inline const Type sign(const Type& a)
{
    if (a < 0)
        return -1;
    else
        return 1;
}


template <class Type>
inline const Type modulo(const Type& a, const Type& b)
{
    Int n = Int(a/b);
    return (Type)(a - n*b);
}



inline DWORD 
floatToDWORD(Float f)             { return *((DWORD*)&f); }


Int random(Int max = 100);

#endif /* __COMMON_ALGORITHM_H__ */
