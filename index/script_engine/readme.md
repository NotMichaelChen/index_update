# Scripting Syntax

## Rules

* Case insensitive
* One command per line
* Arguments are separated by spaces
* No whitespace allowed around each line apart from newlines
* Only one index object can be loaded in

## Commands

RESET
>Resets the current index, clearing out all documents, static info, and other metadata

INSERT *x*
>Inserts x documents. If there aren't enough documents this will insert the remaining documents

QUERY *words*
>Queries the index with the list of words. *words* is separated by spaces

LOAD *filename*
>Loads the index from some file. If a file is already loaded then nothing happens

DUMP *filename*
>Dumps the index to the given file.

LOOP *x*
>Loops the next amount of command x times. Cannot be nested

ENDLOOP
>Specifies end of loop