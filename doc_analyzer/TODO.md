# To Do

This is a non-exhaustive list of the things that still need to be done or are incomplete as of the end of the summer research program

* The Structures sub-module should probably be its own module
* The Redis implementation of the Structures does not have any extra features such as dump-to-disk or restore from backup
* The main interface of the module (indexUpdate) is currently unimplemented despite having all of the necessary functions already implemented
* The Matcher module currently uses the heap to store common blocks, which may not be completely necessary
* The main index currently does not have any functions to dump version exclusive terms
* The decision to make the document store and translation table structures global or parameters into the indexUpdate function is not yet decided.
* The externalpostings header file is used in both the index and doc_analyzer modules. Some restructuring is needed to prevent file duplication
