# dblayer — Slotted-Page Record Layer for toydb

A record management layer built on top of **toydb**, implementing slotted-page storage for variable-length records. Developed as part of CS631 (Database Systems), Assignment 1.

## Overview

`dblayer` sits between the raw page/file layer of toydb and higher-level table operations. It manages how records are packed into fixed-size pages using a **slotted-page** layout, allowing variable-length records to be inserted, deleted, and retrieved efficiently without wasting space or requiring records to shift on every operation.

## Features

- Slotted-page record storage (variable-length records per page)
- Record insertion, deletion, and lookup by record ID
- Page-level space management (free space tracking, compaction)
- Encoding/decoding of records to/from raw byte layout
- CSV data loading into the database
- Database dump/inspection utility

## Project Structure

```
dblayer/
├── codec.c / codec.h    # Record encoding and decoding
├── tbl.c / tbl.h        # Table and slotted-page management
├── util.c / util.h      # Shared utility functions
├── loaddb.c             # Loads records from CSV into the database
├── dumpdb.c             # Dumps database contents for inspection
├── data.csv             # Sample input data
└── makefile             # Build configuration
```

## Building

```bash
make
```

## Usage

Load data into the database:

```bash
./loaddb data.csv <db-file>
```

Dump database contents:

```bash
./dumpdb <db-file>
```

## Design Notes

Each page uses a slot directory at the end of the page, growing toward the middle, while record data is packed from the start of the page, growing toward the slot directory. This allows:

- O(1) access to any record via its slot number
- Records to be deleted by marking slots as empty (tombstoning) without immediately compacting
- Space reuse via compaction when needed

## Course Context

This project was built for **CS631: Database Systems** as Assignment 1, focused on understanding and implementing low-level record storage on top of a toy database engine (`toydb`).

## License

Educational project — for coursework purposes.
