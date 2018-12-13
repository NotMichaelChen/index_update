# Winnowing

## Winnowing procedure
Given a page with all HTML tags and Javascripts removed,
```py
init array F[n] of unsigned char
for each word in the page:
    F[i] = hash(word)

choose b = any value between 10 to 20
init array H[n-b]
for i = 0..n-b:
    H[i] = hash(F[i..i+b-1])

choose WB = 100 or more
init pointcut array C[n-b] all 0s
# Slide the window over H
for w = 0..n-b-WB:
    init array of possible cuts PC
    # Find all possible cuts
    for i = w..w+WB-1:
        cut = true
        for j = w..w+WB-1:
            if i == j:
                continue
            if H[i] > H[j]:
                cut = false
                break
        if cut:
            PC.insert(i)

    # If H[i] is strictly smaller than all other H[j], cut right before F[i]
    if len(PC) == 1:
        C[PC[0]] = 1
    # If we have previously cut directly before one of these positions, continue
    # Otherwise, cut before the rightmost position
    else:
        if checkNoCut(PC) == False:
            C[PC[-1]] = 1

```

## Index building
```py
for each page in the collection:
    split the page into fragments using the winnowing approach

    if URL exists in PageTable:
        update VersionInfo vector in DocVersionTable of the existing docID
    else:
        add new entry to PageTable
        add new VersionInfo to DocVersionTable

    for each fragment:
        calculate the MD5 hash of the fragment content
        if the hash exists in the FragmentHashTable:
            continue
        add a new entry (fragID, hash value, fragSize) to FragmentHashTable
        for each term in the fragment:
            generate the posting following the format (fragID, freq in fragment, vector of positions in the fragment)
```