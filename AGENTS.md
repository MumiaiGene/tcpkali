# AGENTS.md

Guidance for AI agents and contributors working in this repository.

## Project Overview

This is a C project built with GNU Autotools. The `tcpkali` binary is built
from sources under `src/`, with vendored dependencies under `deps/`, ASN.1
codec sources under `asn1/`, documentation under `doc/`, and tests under
`test/`.

The build system is maintained through Autotools input files such as
`configure.ac`, `Makefile.am`, and source-level macro files under `m4/` and
`deps/`.

## Build Workflow

From a fresh git checkout, generate the Autotools build files first:

```sh
autoreconf -iv
./configure
make
```

Run tests with:

```sh
make check
```

At minimum, verify build-system or source changes with:

```sh
autoreconf -iv
./configure
make
```

## Build Dependencies

The build expects typical Autotools and C toolchain packages:

- `autoconf`
- `automake`
- `libtool`
- `pkg-config`
- `bison` or `byacc`
- OpenSSL development headers
- ncurses development headers

Do not assume a fresh checkout contains generated files such as `configure` or
`Makefile.in`; this repository ignores those outputs.

## Autotools File Rules

Autotools input files must be committed when they are referenced by the build.
In particular, files included by `m4_include(...)` are required inputs for
`autoreconf` and must exist in the repository.

Commit these source-level build inputs:

- `configure.ac`
- `Makefile.am`
- `m4/ax_*.m4`
- `deps/libstatsd/libstatsd.m4`

Do not commit generated build outputs:

- `configure`
- `aclocal.m4`
- `config.h.in`
- `Makefile.in`
- `Makefile`
- `config/`
- `autom4te.cache/`
- `libtool`
- `.deps/`
- `.libs/`
- `*.o`
- `*.lo`
- `*.la`

If `autoreconf` reports that an `m4_include(...)` file does not exist, check
whether that `.m4` file is tracked by git before treating it as disposable
generated output.

## Module Ownership

Place new functionality near the existing module that owns the behavior.

- `src/tcpkali.c`, `src/tcpkali.h`: CLI entry point, top-level option parsing,
  and process startup wiring.
- `src/tcpkali_run.c`, `src/tcpkali_run.h`: high-level run orchestration after
  options are parsed.
- `src/tcpkali_engine.c`, `src/tcpkali_engine.h`: event-loop coordination, load
  generation lifecycle, and main runtime engine behavior.
- `src/tcpkali_connection.c`, `src/tcpkali_connection.h`: per-connection state
  and connect/read/write lifecycle.
- `src/tcpkali_transport.c`, `src/tcpkali_transport.h`: socket/transport
  abstraction and network I/O plumbing.
- `src/tcpkali_ssl.c`, `src/tcpkali_ssl.h`: TLS/OpenSSL behavior.
- `src/tcpkali_dns.c`, `src/tcpkali_dns.h`: DNS resolution.
- `src/tcpkali_data.c`, `src/tcpkali_data.h`: payload data generation and
  request body handling.
- `src/tcpkali_websocket.c`, `src/tcpkali_websocket.h`: WebSocket framing and
  protocol behavior.
- `src/tcpkali_expr.*`: expression parser, lexer, and expression evaluation.
  Do not manually reformat generated parser or lexer files.
- `src/tcpkali_regex.c`, `src/tcpkali_regex.h`: regex matching and related
  randomization helpers.
- `src/tcpkali_ring.c`, `src/tcpkali_ring.h`: ring buffer implementation.
- `src/tcpkali_statsd.c`, `src/tcpkali_statsd.h`: statsd integration.
- `src/tcpkali_logging.c`, `src/tcpkali_logging.h`: logging helpers and output
  formatting.
- `src/tcpkali_traffic_stats.h`, `src/tcpkali_rate.h`, `src/tcpkali_mavg.h`:
  statistics and rate calculation helpers.
- `src/tcpkali_iface.c`, `src/tcpkali_iface.h`: network interface handling.
- `src/tcpkali_syslimits.c`, `src/tcpkali_syslimits.h`: OS limits and platform
  capability checks.
- `src/tcpkali_signals.c`, `src/tcpkali_signals.h`: signal handling.
- `src/tcpkali_terminfo.c`, `src/tcpkali_terminfo.h`: terminal capability and
  output support.
- `asn1/`: ASN.1 protocol definitions and generated/support codec files.
- `deps/`: vendored third-party dependencies. Avoid changing these unless the
  fix is specifically for the vendored library.
- `test/`: integration or project-level tests.

When adding a new `.c` or `.h` file under `src/`, update `src/Makefile.am` so
the file is included in `tcpkali_SOURCES` or the relevant test target.

Prefer adding the core implementation to the owning module and keeping callers
thin. For example, CLI flags belong in `tcpkali.c`, but the runtime behavior
they control should usually live in `tcpkali_run.c`, `tcpkali_engine.c`, or the
specific subsystem module.

## Coding Guidelines

- Preserve the existing C style in nearby code.
- Keep changes scoped to the relevant module.
- Avoid unrelated formatting changes.
- Do not manually edit generated parser/lexer outputs unless the project
  already expects those files to be maintained directly.
- Be cautious when changing vendored code under `deps/`; prefer local wrapper
  changes when possible.

## Git Hygiene

Build commands produce many ignored files. Before committing, inspect both
tracked and ignored state:

```sh
git status --short --ignored
git diff --cached --stat
```

Do not force-add generated build outputs. Only force-add ignored files when
they are required source inputs, such as tracked Autotools macro files.
