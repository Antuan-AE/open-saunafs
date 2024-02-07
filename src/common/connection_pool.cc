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

#include "common/platform.h"
#include "common/connection_pool.h"

#include "common/massert.h"
#include "common/sockets.h"

void ConnectionPool::putConnection(int fd, const NetworkAddress& address, int timeout) {
	sassert(fd > 0);
	sassert(timeout > 0);
	std::unique_lock<std::mutex> lock(mutex_);
	connections_[address].push_back(Connection(fd, timeout));
}

int ConnectionPool::getConnection(const NetworkAddress& address) {
	while (true) {
		std::unique_lock<std::mutex> lock(mutex_);
		auto connectionsIterator = connections_.find(address);
		if (connectionsIterator == connections_.end()) {
			return -1;
		}
		std::list<Connection>& openConnections = connectionsIterator->second;
		if (openConnections.empty()) {
			return -1;
		}
		Connection connection = openConnections.front();
		openConnections.pop_front();
		lock.unlock();
		if (connection.isValid()) {
			return connection.fd();
		} else {
			tcpclose(connection.fd());
		}
	}
}

void ConnectionPool::cleanup() {
	std::unique_lock<std::mutex> lock(mutex_);
	std::vector<int> descriptorsToClose;
	ConnectionsContainer::iterator keyAndConnectionListIt = connections_.begin();
	while (keyAndConnectionListIt != connections_.end()) {
		std::list<Connection>& connectionList = keyAndConnectionListIt->second;
		std::list<Connection>::iterator connectionIt = connectionList.begin();
		while (connectionIt != connectionList.end()) {
			if (!connectionIt->isValid()) {
				descriptorsToClose.push_back(connectionIt->fd());
				std::list<Connection>::iterator it = connectionIt;
				++connectionIt;
				connectionList.erase(it);
			} else {
				++connectionIt;
			}
		}
		if (connectionList.empty()) {
			ConnectionsContainer::iterator it = keyAndConnectionListIt;
			++keyAndConnectionListIt;
			connections_.erase(it);
		} else {
			++keyAndConnectionListIt;
		}
	}
	lock.unlock();
	for (int fd : descriptorsToClose) {
		tcpclose(fd);
	}
}
