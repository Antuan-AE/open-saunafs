/*
   Copyright 2013-2015 Skytechnology sp. z o.o.
   Copyright 2023      Leil Storage OÜ

   This file is part of SaunaFS.

   SaunaFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   SaunaFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SaunaFS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/platform.h"

#include <algorithm>

#include "common/compact_vector.h"
#include "common/serialization_macros.h"
#include "master/matotsserv.h"

/// Information about a copy of a file on a tapeserver
struct TapeCopy {

	TapeCopy() : state(TapeCopyState::kInvalid), server(0) {}

	TapeCopy(TapeCopyState state, TapeserverId server) : state(state), server(server) {}

	TapeCopyState state;

	TapeserverId server;
};

/// Information about all copies of a file on tapeservers.
typedef compact_vector<TapeCopy> TapeCopies;
