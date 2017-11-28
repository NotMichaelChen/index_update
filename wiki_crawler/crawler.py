import urllib2, json
import datetime
import os

#Grabs n pages from wikipedia that have at least 4 versions of a page: present, 6 months old, 1 year old, 2 years old
#Each page is then put into a folder of that article's name, and each version of the document is written into the folder
#with the timestamp as the filename

pagecount = input("Enter minimum number of pages to get: ")

if not os.path.exists("dataset"):
    os.makedirs("dataset")
os.chdir("dataset")

pagesobtained = 0
opener = urllib2.build_opener()
opener.addheaders = [('User-Agent', 'DataScrapperForWebResearch/1.0 (mzc223@nyu.edu)')]
while pagesobtained < pagecount:
	#Gets 10 pages to check
    response = opener.open("https://en.wikipedia.org/w/api.php?action=query&list=random&format=json&rnnamespace=0&rnlimit=10&maxlag=5")
    data = json.loads(response.read())

    for i in data["query"]["random"]:
        pageid = i["id"]
        pagetitle = i["title"]
        versions = []
        
        #Get four json's corresponding to current, 6 months, 1 year, and 2 year revision
        datetoget = datetime.date.today()
        dateurl = "https://en.wikipedia.org/w/api.php?action=query&prop=revisions&pageids=" + str(pageid) + "&rvlimit=1&rvprop=content|timestamp&rvdir=older&format=json&maxlag=5"
        for j in range(4):
            response = opener.open(dateurl)
            tempdata = json.loads(response.read())
            versions.append(tempdata)
            if j <= 1:
                datetoget -= datetime.timedelta(6*365/12)
            elif j == 2:
                datetoget -= datetime.timedelta(365)
            dateurl = "https://en.wikipedia.org/w/api.php?action=query&prop=revisions&pageids=" + str(pageid) + "&rvlimit=1&rvprop=content|timestamp&rvdir=newer&rvstart=" + datetoget.isoformat() + "T00:00:00Z&format=json&maxlag=5"
        
        #Get timestamps of four json's
        timestamps = []
        for v in versions:
            #"revisions" doesn't exist if page wasn't edited since date given
            if "revisions" not in v["query"]["pages"][str(pageid)]:
                break
            timestamps.append(v["query"]["pages"][str(pageid)]["revisions"][0]["timestamp"])
        
        
        if len(timestamps) != 4 or (timestamps[0] == timestamps[1] or timestamps[1] == timestamps[2] or timestamps[2] == timestamps[3]):
            continue
        else:
            if not os.path.exists(pagetitle):
                os.makedirs(pagetitle)
            #If we already got that page, don't get it again
            else:
                continue
            os.chdir(pagetitle)
            
            for j in range(4):
                #Colons are illegal windows characters, so replace them with semicolons
                f = open(timestamps[j].replace(":", ";") + ".txt", "w")
                f.write(versions[j]["query"]["pages"][str(pageid)]["revisions"][0]["*"].encode('utf-8').strip())
                f.close()
                
            os.chdir("..")
            pagesobtained += 1
    
    print str(pagesobtained) + " pages obtained"

#Get random wikipedia page
#Check if it has distinct pages for 6 months, 1 year, 2 years
#Save 4 versions if it does
#Repeat until saved desired amount of pages
