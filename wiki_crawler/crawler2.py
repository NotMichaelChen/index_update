import urllib2, json
import datetime
import os
import re

def removeNonAscii(s):
    return "".join(i for i in s if ord(i)<128)

def replaceWhitespace(s):
    return re.sub(r'\s+', '_', s.strip())

def formatArticle(doc):
    s1 = re.sub(r'[^a-zA-Z\d\s]', ' ', doc)
    return re.sub(r'[ \t]+', ' ', s1).strip()

#Grabs n pages from wikipedia that have at least 4 versions of a page
#Each iteration of the loop puts all obtained documents into one .raw file

pagecount = input("Enter minimum number of pages to get: ")

if not os.path.exists("dataset"):
    os.makedirs("dataset")
os.chdir("dataset")

#Count how many valid pages were obtained
pagesobtained = 0
#Count which file we're writing to: this is used to number the .raw files
filecount = 0
opener = urllib2.build_opener()
opener.addheaders = [('User-Agent', 'DataScrapperForWebResearch/1.0 (mzc223@nyu.edu)')]

#Files are 4-digits
ofile = open("{}.raw".format(str(filecount).zfill(4)), 'w')
while pagesobtained < pagecount:
    #Gets some pages to check, returns JSON containing a list of random page titles/ids
    response = opener.open("https://en.wikipedia.org/w/api.php?action=query&list=random&format=json&rnnamespace=0&rnlimit=100&maxlag=5")
    data = json.loads(response.read())

    #For each page...
    for i in data["query"]["random"]:
        pageid = i["id"]
        pagetitle = i["title"]
        versions = []

        #JSON containing the 5 most recent revisions of the page
        response = opener.open("https://en.wikipedia.org/w/api.php?action=query&format=json&prop=revisions&pageids=" + str(pageid) + "&rvprop=content|timestamp&rvlimit=5")
        revisions = json.loads(response.read())

        for j in revisions["query"]["pages"]:
            for k in revisions["query"]["pages"][j]["revisions"]:
                if("*" not in k.keys()):
                    continue
                #Write magic number, timestamp, url/title
                ofile.write("df6fa1abb58549287111ba8d776733e9 {} {}\n".format(k["timestamp"], replaceWhitespace(removeNonAscii(pagetitle))))
                #Write article
                ofile.write(formatArticle(k["*"]) + "\n")
        
        pagesobtained += 1
        if(pagesobtained % 1000 == 0):
            ofile.close()
            filecount += 1
            ofile = open("{}.raw".format(str(filecount).zfill(4)), 'w')

    print str(pagesobtained) + " pages obtained"

#Get random wikipedia page
#Check if it has distinct pages for 6 months, 1 year, 2 years
#Save 4 versions if it does
#Repeat until saved desired amount of pages
