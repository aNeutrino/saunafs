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

#include <memory>
#include <string>

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "common/exception.h"

/// Exception thrown by methods of the \ref TlsSession class.
SAUNAFS_CREATE_EXCEPTION_CLASS(GnutlsException, Exception);

/// A wrapper for a TLS session.
class TlsSession {
public:
	/// A constant which can be used in \ref TlsSession::TlsSession for unused used.
	static const std::string kNoFile;

	/// Initializes a TLS session.
	///
	/// When the session is initialized, `gnutls_handshake` should be used to establish it.
	///
	/// Any paths in this constructor may be specified as \ref kNoFile. However, the `keyFile` and
	/// `certFile` would be required to successfully establish a session for a server; `trustDir`
	/// would be required to successfully establish a session for a client; all paths are required
	/// when mutual authentication is enabled using `gnutls_certificate_server_set_request`.
	///
	/// \param socket  The network socket to be used by the session.
	/// \param flags  Flags for `gnutls_init`; either `GNUTLS_SERVER` or `GNUTLS_CLIENT`.
	/// \param keyFile  The path to our private key file in the `GNUTLS_X509_FMT_PEM` format.
	/// \param certFile  The path to our certificate file in the `GNUTLS_X509_FMT_PEM` format.
	/// \param trustDir  The path to a directory with trusted CA certificates
	///                  in the `GNUTLS_X509_FMT_PEM` format.
	TlsSession(int socket, unsigned int flags,
			const std::string& keyFile, const std::string& certFile, const std::string& trustDir);

	/// Returns the underlying GnuTLS session.
	gnutls_session_t session() const {
		return *session_;
	}

private:
	/// A deleter for gnutls_certificate_credentials_t.
	///
	/// Calls `gnutls_certificate_free_credentials` and deallocates gnutls_certificate_credentials_t.
	struct CredentialsDeleter {
		void operator() (gnutls_certificate_credentials_t* credentials);
	};

	/// A deleter for gnutls_session_t.
	///
	/// Calls `gnutls_deinit` and deallocates gnutls_session_t.
	struct SessionDeleter {
		void operator() (gnutls_session_t* session);
	};

	/// A RAII wrapper for `gnutls_certificate_credentials_t`.
	typedef std::unique_ptr<gnutls_certificate_credentials_t, CredentialsDeleter> GnuTlsCredentials;

	/// A RAII wrapper for `gnutls_session_t`.
	typedef std::unique_ptr<gnutls_session_t, SessionDeleter> GnuTlsSession;

	/// GNU TLS session data.
	GnuTlsSession session_;

	/// Credentials used with the session.
	GnuTlsCredentials credentials_;
};
