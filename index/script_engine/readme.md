# Scripting Syntax

## Rules

* Case insensitive
* One command per line
* Arguments are separated by spaces
* No whitespace allowed around each line apart from newlines
* Only one index object can be loaded in

## Commands

DOCINPUT *path reader*
>Specify where to read documents, and which documentreader to use

RESET
>Resets the current index, clearing out all documents, static info, and other metadata. Must call SETDIR to resume insertions

INSERT *x*
>Inserts x documents. If there aren't enough documents this will insert the remaining documents

QUERY *words*
>Queries the index with the list of words. *words* is separated by spaces

SETDIR *dir*
>Sets the directory that all files will be written to. Clears the current index

LOADCURRENTDIR
>Loads the currently selected directory. This overwrites the current index

DUMP *filename*
>Dumps the index to the given file. Requires a directory to be set

LOOP *x*
>Loops the next amount of command x times. Cannot be nested

ENDLOOP
>Specifies end of loop