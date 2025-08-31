/*
   Copyright 2005-2010 Jakub Kruszona-Zawadzki, Gemius SA
   Copyright 2013-2014 EditShare
   Copyright 2013-2015 Skytechnology sp. z o.o.
   Copyright 2023      Leil Storage OÜ


   SaunaFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   SaunaFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SaunaFS  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/platform.h"

#include "master/filesystem_freenode.h"

#include "master/filesystem_metadata.h"

inode_t fsnodes_get_next_id(uint32_t ts, inode_t req_inode) {
	const inode_t shardBegin = gMetadata->inodeShardBegin();
	const inode_t shardEnd = gMetadata->inodeShardLimit();

	// If a specific inode is requested, accept only if it is within this shard and available
	if (req_inode != 0) {
		if (req_inode < shardBegin || req_inode > shardEnd ||
		    !gMetadata->inodePool.markAsAcquired(req_inode, ts)) {
			req_inode = 0;  // fall back to auto-allocate
		}
	}

	if (req_inode == 0) {
		// Prefer strictly increasing allocation within our shard using a cheap check+mark
		// to avoid pulling out-of-shard ids from the pool and releasing them.
		inode_t maxSeen = gMetadata->maxInodeId().getValue();
		inode_t cursor = maxSeen;
		// Normalize cursor into [shardBegin-1, shardEnd-1]
		if (cursor + 1 < shardBegin) {
			cursor = shardBegin - 1;
		}
		if (cursor >= shardEnd) {
			// Global max might be beyond our shard; start a hole-scan from shard begin
			cursor = shardBegin - 1;
		}

		bool wrapped = false;
		for (;;) {
			if (cursor >= shardEnd) {
				if (!wrapped) {
					// Single wrap to try holes from the beginning of the shard
					cursor = shardBegin - 1;
					wrapped = true;
					continue;
				}
				mabort("Out of free inode numbers in shard");
			}
			++cursor;
			if (gMetadata->inodePool.markAsAcquired(cursor, ts)) {
				req_inode = cursor;
				break;
			}
		}
	}
	if (req_inode > gMetadata->maxInodeId().getValue()) {
		gMetadata->maxInodeId().setValue(req_inode);
	}

	return req_inode;
}

uint8_t fs_apply_freeinodes(uint32_t /*ts*/, inode_t /*freeinodes*/) {
	// left for compatibility when reading from old metadata change log
	gMetadata->metadataVersion++;
	return 0;
}
