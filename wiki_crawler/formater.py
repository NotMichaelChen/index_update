import re
import os

if not os.path.exists("dataset-format"):
    os.makedirs("dataset-format")

for subdir, dirs, files in os.walk("dataset"):
    for file in files:
        filepath = subdir + os.sep + file
        
        f = open(filepath, "r")
        rawfile = f.read()
        f = open("dataset-format" + os.sep + subdir.split(os.sep)[1] + "~" + file, "w")
        
        tokens = rawfile.split()
        for tok in tokens:
            f.write(re.sub(r'\W+', ' ', tok) + " ")
            
            
