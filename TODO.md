# TODO

## SSL/TLS client gaps versus `openssl s_client`

Current tcpkali TLS client support is intentionally minimal: `--ssl` enables a
TLS client connection, and the client now derives SNI from the first target
hostname when the target is a DNS name. SSL errors are also printed with the
OpenSSL error string instead of a raw numeric error code.

Compared with `openssl s_client`, tcpkali still lacks several client-side TLS
controls and diagnostics.

### Correctness and compatibility

- Add an explicit SNI option.
  - `--ssl-server-name <name>`: override the automatically derived SNI name.
  - `--ssl-no-sni`: disable SNI intentionally.
  - Keep the current default: use the first target hostname when it is not an
    IP literal.
- Add certificate verification controls.
  - `--ssl-verify`: enable peer certificate verification.
  - `--ssl-ca-file <file>` and `--ssl-ca-path <dir>`.
  - `--ssl-verify-hostname <name>` or reuse the SNI hostname by default when
    verification is enabled.
  - Print clear verification failures, including depth, subject, issuer, and
    OpenSSL verify error text.
- Add client certificate authentication for outbound TLS.
  - Reuse or split the current server-side `--ssl-cert` and `--ssl-key`
    behavior so they can also load a client certificate/key.
  - Support encrypted private keys with a passphrase mechanism.
- Add TLS protocol version controls.
  - `--tls-min-version <1.0|1.1|1.2|1.3>`.
  - `--tls-max-version <1.0|1.1|1.2|1.3>`.
  - Consider aliases for common cases, such as `--tls1_2` and `--tls1_3`.
- Add cipher and TLS 1.3 ciphersuite selection.
  - `--ssl-cipher <list>` for pre-TLS-1.3 cipher strings.
  - `--tls-ciphersuites <list>` for TLS 1.3.
- Add ALPN support.
  - `--alpn <proto[,proto...]>`, for example `h2,http/1.1`.
  - Report the negotiated ALPN protocol in verbose output.

### Diagnostics

- Improve TLS handshake failure reporting.
  - Current output now includes the OpenSSL error string, for example
    `sslv3 alert handshake failure`.
  - Also print the remote address and SNI used for the failed connection.
  - Decode common TLS alert numbers into names where OpenSSL does not already
    print a useful string.
- Add verbose TLS session reporting similar to `s_client`.
  - Protocol version.
  - Cipher/ciphersuite.
  - Peer certificate subject and issuer.
  - Verification result.
  - Session reuse status.
- Add optional certificate chain printing.
  - `--ssl-show-certs` or similar.
  - Keep this behind a flag so normal load-test output remains compact.
- Add optional key log support for Wireshark.
  - `--ssl-keylog-file <path>`.
  - Use OpenSSL keylog callback where available.

### Advanced `s_client`-like features

- Add session resumption controls.
  - Save/reuse sessions across connections where appropriate.
  - Report whether a connection reused a session.
- Add OCSP stapling diagnostics.
  - Request status with `SSL_set_tlsext_status_type`.
  - Print whether a stapled response was present.
- Add curve/group and signature algorithm controls if needed for protocol
  compatibility testing.
  - `--ssl-groups <list>`.
  - `--ssl-sigalgs <list>`.
- Consider STARTTLS modes only if tcpkali needs to test protocols such as SMTP,
  IMAP, POP3, or LDAP. This is useful in `s_client`, but outside tcpkali's
  current HTTP/raw TCP load-test focus.

### Tests to add

- Unit or integration test that `--ssl` to a DNS target sends SNI by default.
- Test that an IP-literal target does not send SNI unless explicitly overridden.
- Test that a forced bad SNI produces a readable TLS alert message and does not
  crash.
- Test that TLS handshake failure closes the connection exactly once.
- Regression test for the previous double-free path after SSL handshake
  failure.
