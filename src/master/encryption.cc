/*
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


#include <memory>

#include "common/event_loop.h"
#include "common/gnutls_global_resources.h"
#include "common/main.h"
#include "master/encryption.h"

/// Keeps GnuTLS initialized.
static std::unique_ptr<GnuTlsGlobalResources> gGnutlsGlobalResources;

int encryption_init() {
	gGnutlsGlobalResources.reset(new GnuTlsGlobalResources());

	eventloop_destructregister([]() {
		gGnutlsGlobalResources.reset();
		safs::log_info("GnuTLS deinitialized");
	});

	return 0;
}
