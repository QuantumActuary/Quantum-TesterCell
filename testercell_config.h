/*
 * Copyright (c) Thomas Chen - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Thomas Chen <tkchen@gmail.com>, August 2015
 *
 * logiccell_config.h
 */

#ifndef TESTERCELL_CONFIG_H_
#define TESTERCELL_CONFIG_H_

// ------------------------------------------------------------------------- //

// Platform recognition
#if defined(WIN32) || defined(_WIN32)
  #define TESTERCELLPLUGIN_WIN32 1
#elif defined(__linux)
  #define TESTERCELLPLUGIN_LINUX 1
#elif defined(__APPLE__)
  #define TESTERCELLPLUGIN_OSX 1
#else
  //unsupported OS
#endif

// ------------------------------------------------------------------------- //

// Decides whether symbols are imported from a dll (client app) or exported to
// a dll (MyEngine library). The MYENGINE_SOURCE symbol is defined by
// all source files of the library, so you don't have to worry about a thing.
#if defined(_MSC_VER)

  #if defined(TESTERCELLPLUGIN_STATICLIB)
    #define TESTERCELL_API
  #else
    #if defined(TESTERCELLPLUGIN_SOURCE)
      // If we are building the DLL, export the symbols tagged like this
      #define TESTERCELL_API __declspec(dllexport)
    #else
      // If we are consuming the DLL, import the symbols tagged like this
      #define TESTERCELL_API __declspec(dllimport)
    #endif
  #endif

#elif defined(__GNUC__)

  #if defined(TESTERCELLPLUGIN_STATICLIB)
    #define TESTERCELL_API
  #else
    #if defined(TESTERCELLPLUGIN_SOURCE)
      #define TESTERCELL_API __attribute__ ((visibility ("default")))
    #else
      // If you use -fvisibility=hidden in GCC, exception handling and RTTI
      // would break if visibility wasn't set during export _and_ import
      // because GCC would immediately forget all type infos encountered.
      // See http://gcc.gnu.org/wiki/Visibility
      #define TESTERCELL_API __attribute__ ((visibility ("default")))
    #endif
  #endif

#else

  #error Unknown compiler, please implement shared library macros

#endif

#endif /* TESTERCELL_CONFIG_H_ */
