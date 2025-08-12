/*
   Copyright 2023 Leil Storage OÜ

   This file is part of SaunaFS.

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

#pragma once

#include "common/platform.h"

#include <gnutls/gnutls.h>
#include <syslog.h>

#include "common/exceptions.h"

/// A RAII wrapper for the initialization of GnuTLS global resources.
class GnuTlsGlobalResources {
public:
	/// Initializes GnuTLS.
	///
	/// \param loglevel  The level is an integer between 0 and 99 passed to
	/// \ref gnutls_global_set_log_level. Higher values mean more verbosity.
	/// The default value is 1 which enables most important messages (e.g.,
	/// information about problems with certificate verification) without
	/// logging most of GnuTLS debugging information.
	/// \throws InitializeException if GnuTLS cannot be initialized.
	GnuTlsGlobalResources(int loglevel = 1) {
		int ret = gnutls_global_init();
		if (ret < 0) {
			throw InitializeException("gnutls initialization error "
					+ std::string(gnutls_strerror(ret)));
		}

		// Configure Gnu TLS to log important information.
		gnutls_global_set_log_function([](int level, const char* msg) {
			syslog(LOG_INFO, "GnuTLS [level=%d] %s", level, msg);
		});
		gnutls_global_set_log_level(loglevel);
	}

	/// Deinitializes GnuTLS.
	~GnuTlsGlobalResources() {
		gnutls_global_deinit();
	}
};
