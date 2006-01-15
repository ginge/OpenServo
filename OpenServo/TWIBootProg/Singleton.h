// Copyright (C)2004 Dimax ( http://www.xdimax.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

template <class T>
    class CSingletonDestroyer
    {
    public:
        CSingletonDestroyer() {};
        ~CSingletonDestroyer()
        {
            CSingleton<T>::DestroyObject();
        }
        void Instantiate() {};
    };


template <class T>
class CSingleton
{
public:
    static T* Instance();
private:

    friend class CSingletonDestroyer<T>;

    
    static T* sm_pInstance;
    static CSingletonDestroyer<T> sm_Destroyer;

    static void DestroyObject()
    {
        if (sm_pInstance != NULL)
            delete sm_pInstance;
    }
};


template <class T>
CSingletonDestroyer<T> CSingleton<T>::sm_Destroyer = CSingletonDestroyer<T>();



template <class T>
T* CSingleton<T>::Instance()
{
    if (sm_pInstance == NULL)
        sm_pInstance = new T;
    CSingleton::sm_Destroyer.Instantiate();
    return sm_pInstance;
}

template <class T>
T* CSingleton<T>::sm_pInstance = NULL;

