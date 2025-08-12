#include "common/tls_session.h"

#include <dirent.h>
#include <syslog.h>

#include "common/cwrap.h"

const std::string TlsSession::kNoFile = "/unused file/";

/// Callback for GnuTLS which verifies certificates for the given session.
static int verify_certificates(gnutls_session_t session) {
	unsigned int status;
	int ret = gnutls_certificate_verify_peers2(session, &status);
	if (ret < 0) {
		syslog(LOG_WARNING,
				"error checking TLS certificate: %s", gnutls_strerror(ret));
		return GNUTLS_E_CERTIFICATE_ERROR;
	} else if (status != 0) {
		if (status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
			syslog(LOG_WARNING,
					"TLS certificate could not be verified against known CAs");
		} else {
			syslog(LOG_WARNING,
					"TLS certificate verification failed (error #%u)", status);
		}
		return GNUTLS_E_CERTIFICATE_ERROR;
	} else {
		return GNUTLS_E_SUCCESS;
	}
}

TlsSession::TlsSession(int socket, unsigned int flags,
		const std::string& keyFile, const std::string& certFile, const std::string& trustDir) {
	// Set up credentials used for this connection
	credentials_.reset(new gnutls_certificate_credentials_t());
	int ret = gnutls_certificate_allocate_credentials(credentials_.get());
	if (ret < 0) {
		delete credentials_.release();
		throw GnutlsException("gnutls certificate allocation failed: "
				+ std::string(gnutls_strerror(ret)));
	}
	if (keyFile != kNoFile && certFile != kNoFile) {
		ret = gnutls_certificate_set_x509_key_file(*credentials_,
				certFile.c_str(), keyFile.c_str(), GNUTLS_X509_FMT_PEM);
		if (ret < 0) {
			throw GnutlsException("gnutls certificate key/cert file failed: "
					+ std::string(gnutls_strerror(ret)));
		}
	}
	if (trustDir != kNoFile) {
		// We have to add all certificates from `trustDir` to `credentials_`. Unfortunately,
		// GnuTLS on Ubuntu 12.04 is old and doesn't support gnutls_certificate_set_x509_trust_dir
		// which makes us implement it manually here.
		cdirectory_t dirHandle(opendir(trustDir.c_str()));
		if (dirHandle == nullptr) {
			throw GnutlsException("cannot open trusted directory '" + trustDir + "': "
					+ errorString(errno));
		}

		struct dirent* entry;
		int numberOfCertificates = 0;
		while ((errno = 0, entry = readdir(dirHandle.get()))) {
			std::string certPath = trustDir + "/" + entry->d_name;
			ret = gnutls_certificate_set_x509_trust_file(*credentials_,
					certPath.c_str(), GNUTLS_X509_FMT_PEM);
			numberOfCertificates += (ret < 0 ? 0 : ret);
		}
		if (errno) {
			throw GnutlsException("cannot read trusted directory '" + trustDir + "': "
					+ errorString(errno));
		}
		if (numberOfCertificates == 0) {
			throw GnutlsException("trusted directory '" + trustDir + "' contains no certificates");
		}
	}
	gnutls_certificate_set_verify_function(*credentials_, verify_certificates);

	// Set up session used for this connection
	session_.reset(new gnutls_session_t());
	ret = gnutls_init(session_.get(), flags | GNUTLS_NONBLOCK);
	if (ret < 0) {
		delete session_.release();
		throw GnutlsException("gnutls session initialization failed: "
				+ std::string(gnutls_strerror(ret)));
	}

	ret = gnutls_credentials_set(*session_, GNUTLS_CRD_CERTIFICATE, *credentials_);
	if (ret < 0) {
		throw GnutlsException("gnutls setting credentials failed: "
				+ std::string(gnutls_strerror(ret)));
	}

	// Botan doesn't implement ECDHE-RSA correctly. Disable this algorithm.
	ret = gnutls_priority_set_direct(*session_, "NORMAL:!ECDHE-RSA", NULL);
	if (ret < 0) {
		throw GnutlsException("gnutls setting priorities failed: "
				+ std::string(gnutls_strerror(ret)));
	}

	gnutls_transport_set_ptr(*session_, (gnutls_transport_ptr_t) (size_t) socket);
}

void TlsSession::CredentialsDeleter::operator() (gnutls_certificate_credentials_t* credentials) {
	gnutls_certificate_free_credentials(*credentials);
	delete credentials;
}

void TlsSession::SessionDeleter::operator() (gnutls_session_t* session) {
	gnutls_deinit(*session);
	delete session;
}
