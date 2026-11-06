# vCard Manager

A terminal-based contact-management application with a **C vCard parser** and a **Python interface**. It works with `.vcf` contact files and includes MySQL integration.

**Stack:** C · Python · MySQL · GNU Make

## Features

- Parse and work with vCard (`.vcf`) contact information.
- Create, load, and update contact records.
- Use a Python terminal interface built with Asciimatics.
- Work with contact data through the underlying C library.

## Architecture

```text
Python interface (bin/A3main.py)
        |
        v
C shared library (bin/libvcparser.so)
        |
        v
vCard parser and linked-list utilities (src/)
```

The project contains C source under `src/`, headers in `include/`, and the Python interface in `bin/`.

## Build

From the repository root, using a Unix-like development environment:

```bash
make
```

This builds `bin/libvcparser.so`. The Python interface also requires its Python dependencies and a configured local MySQL environment.

```bash
cd bin
python3 A3main.py
```

## Notes

This is a desktop/terminal development project, not a hosted web service. Database configuration and platform-specific dependencies may need adjustment on another computer.

## Potential improvements

- Automated parser and integration tests
- Clearer dependency and database setup
- More robust validation and error reporting
