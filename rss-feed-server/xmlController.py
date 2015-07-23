import xml.etree.ElementTree as ET
import os
from email.Utils import formatdate
from flask import Flask
from flask import Flask, request

app = Flask(__name__)


# userID should be passed in as a string with no spaces
def getFileNameForUser(userID):
    extension = ".xml"
    return "static/" + userID + extension


def createNewXml(userID, action):
    newRoot = ET.Element("rss")
    newRoot.set('version', '2.0')
    channel1 = ET.SubElement(newRoot, "channel")
    makeFirstItem(channel1, action, userID)
    newnewTree = ET.ElementTree(newRoot)
    newnewTree.write(getFileNameForUser(userID), encoding="UTF-8")


def makeFirstItem(channel1, action, userID):
    ET.SubElement(channel1, "title").text = "RSS Feed for IFTTT"
    ET.SubElement(channel1, "link").text = "http://rosexu.github.io/BattleHackRSS/" + userID
    ET.SubElement(channel1, "description").text = "Personalized RSS for user " + userID
    newItem = ET.SubElement(channel1, "item")
    populateSubfields(newItem, action, "http://rosexu.github.io/BattleHackRSS/" + userID, "Personalized RSS for user " + userID, getCurrentTime(), "http://rosexu.github.io/BattleHackRSS/" + userID + "1")


def populateSubfields(item, title, link, desc, date, guid):
    ET.SubElement(item, "title").text = title
    ET.SubElement(item, "link").text = link
    ET.SubElement(item, "description").text = desc
    ET.SubElement(item, "pubDate").text = date
    ET.SubElement(item, "guid").text = guid


@app.route("/addtoxml", methods=['POST'])
def postXmlData():
    userID = request.form.get("userid")
    action = request.form.get("action")
    app.logger.info(userID)
    app.logger.info(action)
    handleRequest(userID, action)
    return "success"


def handleRequest(userID, action):
    try:
        fileName = getFileNameForUser(userID)
        addNewItem(fileName, action, "link", "description", getCurrentTime(), getNewGuid(fileName, getLastChildIndex(fileName)))
        print "file exists"
    except IOError:
        createNewXml(userID, action)


@app.route("/")
def hello():
    print "hello"
    return "hello!"


def getChannel(fileName):
    tree = ET.parse(fileName)
    root = tree.getroot()

    # tree1 = ET.parse('haha.xml')

    return root[0]


def getCurrentTime():
    return formatdate()


def getLastChildIndex(fileName):
    channel = getChannel(fileName)
    index = 0
    for child in channel:
        index += 1
    return index


def getNewGuid(fileName, index):
    channel = getChannel(fileName)
    oldGuid = channel[index-1].find("guid")
    try:
        lastCharOfOldGuid = int(oldGuid.text[-1:])
        newGuidNum = lastCharOfOldGuid + 1
        newGuid = oldGuid.text[:-1] + str(newGuidNum)
        return newGuid
    except ValueError:
        print "error: last char is not a number"


def addNewItem(fileName, title, link, description, pubDate, guid):
    tree = ET.parse(fileName)
    root = tree.getroot()

    # tree1 = ET.parse('haha.xml')

    channel = root[0]
    print ET.tostring(channel)
    item = ET.SubElement(channel, "item")
    populateSubfields(item, title, link, description, pubDate, guid)
    print ET.tostring(tree.getroot())
    tree.write(fileName, encoding="UTF-8")

# this adds a new item to Test.xml(RSS feed) with child tags of the things you passes in.
# addNewItem("turn music off", "link", "description", getCurrentTime(), getNewGuid(getLastChildIndex()))
# makes a new xml file for first time users
# createNewXml("20565628", "make music")
# handleRequest("20565627", "make candy")

if __name__ == "__main__":
    port = int(os.environ.get('PORT', 5000))
    app.run(debug=True, host='0.0.0.0', port=port)
