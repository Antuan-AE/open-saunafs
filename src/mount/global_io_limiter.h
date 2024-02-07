/*
   Copyright 2013-2014 EditShare
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

#include <functional>

#include "common/io_limiting.h"
#include "mount/mastercomm.h"

namespace ioLimiting {

// An object used locally in mount that communicates with a global limiter
// running in master
struct MasterLimiter : public Limiter {
	MasterLimiter();
	~MasterLimiter();
	// Passes a request to master
	uint64_t request(const IoLimitGroupId& groupId, uint64_t size) override;
private:
	class IolimitsConfigHandler : public PacketHandler {
	public:
		IolimitsConfigHandler(MasterLimiter& parent) : parent_(parent) {}

		bool handle(MessageBuffer buffer);
	private:
		MasterLimiter& parent_;
	};

	IolimitsConfigHandler iolimitsConfigHandler_;
	uint32_t configVersion_;
};

// The local limiter running in this mount instance (used in local I/O limiting)
struct MountLimiter : public Limiter {
	uint64_t request(const IoLimitGroupId& groupId, uint64_t size) override;
	void loadConfiguration(const IoLimitsConfigLoader& config);
private:
	IoLimitsDatabase database_;
};

// This class is a proxy that locally handles calls to a possibly remote Limiter.
// It classifies clients into groups and performs required delays.
class LimiterProxy {
public:
	LimiterProxy(Limiter& limiter, Clock& clock) :
		shared_(limiter, std::chrono::milliseconds(100)),
		enabled_(true),
		clock_(clock)
	{
		using namespace std::placeholders;
		limiter.registerReconfigure(std::bind(
			&LimiterProxy::reconfigure,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3));
	}

	// Try to acquire an assignment of 'size' bytes. This method pauses a callee until a request
	// is satisfied or a deadline is exceeded. Return when returns SFS status
	uint8_t waitForRead(const pid_t pid, const uint64_t size, SteadyTimePoint deadline);
	// Works the same as waitForRead
	uint8_t waitForWrite(const pid_t pid, const uint64_t size, SteadyTimePoint deadline);

private:
	typedef std::map<IoLimitGroupId, std::shared_ptr<Group>> Groups;

	std::shared_ptr<Group> getGroup(const IoLimitGroupId& groupId) const;
	// Remove groups that were deleted, cancel queued operations assigned to them. Add new groups.
	// Update the delta_us parameter.
	// If subsystem was changed, cancel all queued operations and removed groups that were used.
	void reconfigure(uint32_t delta_us, const std::string& subsystem,
			const std::vector<IoLimitGroupId>& groupIds);

	std::mutex mutex_;
	SharedState shared_;
	std::string subsystem_;
	Groups groups_;
	bool enabled_;
	Clock& clock_;
};

} // namespace ioLimiting
