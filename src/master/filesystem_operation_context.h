/*
   Copyright 2025      Leil Storage OÜ

   This file is part of SaunaFS.

   SaunaFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   SaunaFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SaunaFS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/platform.h"

#include <memory>

namespace kv {
class IReadOnlyTransaction;
class IReadWriteTransaction;
}  // namespace kv

/// Context for filesystem operations that may require database transactions.
/// This object carries the current transaction and operation metadata.
/// Master ignores the transaction, MDS uses it for FDB access.
class FilesystemOperationContext {
public:
	/// Type of transaction required for this operation
	enum class TransactionType : std::uint8_t {
		kReadOnly,  ///< Read-only transaction
		kReadWrite  ///< Read-write transaction
	};

	/// Create an empty context (no transaction)
	FilesystemOperationContext() = default;

	/// Create a context with a read-only transaction
	explicit FilesystemOperationContext(std::unique_ptr<kv::IReadOnlyTransaction> txn);

	/// Create a context with a read-write transaction
	explicit FilesystemOperationContext(std::unique_ptr<kv::IReadWriteTransaction> txn);

	/// Move-only semantics (transactions cannot be copied)
	FilesystemOperationContext(const FilesystemOperationContext &) = delete;
	FilesystemOperationContext &operator=(const FilesystemOperationContext &) = delete;
	FilesystemOperationContext(FilesystemOperationContext &&) = default;
	FilesystemOperationContext &operator=(FilesystemOperationContext &&) = default;

	/// Destructor
	~FilesystemOperationContext() = default;

	/// Returns true if this context has an active transaction
	bool hasTransaction() const;

	/// Returns true if this context has a read-write transaction
	bool hasReadWriteTransaction() const;

	/// Get the read-only transaction (null if none)
	kv::IReadOnlyTransaction *getReadOnlyTransaction() const;

	/// Get the read-write transaction (null if none or read-only)
	kv::IReadWriteTransaction *getReadWriteTransaction() const;

	/// Set the transaction type hint for lazy creation
	void setTransactionType(TransactionType type) { transactionType_ = type; }

	/// Get the transaction type hint
	TransactionType getTransactionType() const { return transactionType_; }

private:
	/// The active read-write transaction (if any)
	std::unique_ptr<kv::IReadWriteTransaction> rwTransaction_;

	/// The active read-only transaction (if any and rwTransaction_ is null)
	std::unique_ptr<kv::IReadOnlyTransaction> roTransaction_;

	/// Hint about what transaction type to create if lazy initialization is needed
	TransactionType transactionType_ = TransactionType::kReadWrite;
};
