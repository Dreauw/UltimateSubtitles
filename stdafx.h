// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <atlimage.h>
#include <atlbase.h>
#include <atlcom.h>
#include <objbase.h>
#include <shlwapi.h>
#include <shlobj.h> 
#include <ole2.h>
#include <objidl.h>
#include <gdiplus.h>
#include <wininet.h>
#include <Mlang.h>


// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>

// C++
#include <fstream>
#include <sstream>
#include <istream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstdarg>
#include <string>
#include <vector>
#include <set>

// Boost
#include <boost\format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

// Zlib
#include <zlib.h>

// TODO: reference additional headers your program requires here
void print(char* str, ...);