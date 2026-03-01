#pragma once

// Compatibility shim: some legacy project settings still reference stdafx.h for /Yc and /Yu.
// Keep this forwarding header so both stdafx.h and pch.h based setups work.
#include "pch.h"
