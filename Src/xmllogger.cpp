#include "xmllogger.h"
#include <iostream>

using tinyxml2::XMLElement;
using tinyxml2::XMLNode;

bool XmlLogger::getLog(const char *FileName, const std::string *LogParams)
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD) return true;

    if (doc.LoadFile(FileName) != tinyxml2::XMLError::XML_SUCCESS) {
        std::cout << "Error opening input XML file" << std::endl;
        return false;
    }

    if (LogParams[CN_LP_PATH] == "" && LogParams[CN_LP_NAME] == "") {
        std::string str;
        str.append(FileName);
        size_t found = str.find_last_of(".");
        if (found != std::string::npos)
            str.insert(found, "_log");
        else
            str.append("_log");
        LogFileName.append(str);
    } else if (LogParams[CN_LP_PATH] == "") {
        LogFileName.append(FileName);
        std::string::iterator it = LogFileName.end();
        while (*it != '\\')
            it--;
        ++it;
        LogFileName.erase(it, LogFileName.end());
        LogFileName.append(LogParams[CN_LP_NAME]);
    } else if (LogParams[CN_LP_NAME] == "") {
        LogFileName.append(LogParams[CN_LP_PATH]);
        if (*(--LogParams[CN_LP_PATH].end()) != '\\') LogFileName.append("\\");
        std::string lfn;
        lfn.append(FileName);
        size_t found = lfn.find_last_of("\\");
        std::string str = lfn.substr(found);
        found = str.find_last_of(".");
        if (found != std::string::npos)
            str.insert(found, "_log");
        else
            str.append("_log");
        LogFileName.append(str);
    } else {
        LogFileName.append(LogParams[CN_LP_PATH]);
        if (*(--LogParams[CN_LP_PATH].end()) != '\\') LogFileName.append("\\");
        LogFileName.append(LogParams[CN_LP_NAME]);
    }

    XMLElement *log, *root = doc.FirstChildElement(CNS_TAG_ROOT);

    if (!root) {
        std::cout << "No '" << CNS_TAG_ROOT << "' element found in XML file" << std::endl;
        std::cout << "Can not create log" << std::endl;
        return false;
    }

    root->InsertEndChild(doc.NewElement(CNS_TAG_LOG));

    root = (root->LastChild())->ToElement();

    return true;
}

void XmlLogger::saveLog()
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD)
        return;
    doc.SaveFile(LogFileName.c_str());
}

void XmlLogger::writeToLogNewResult() {
    if (loglevel == CN_LP_LEVEL_NOPE_WORD)
        return;

    XMLElement *mapTag = doc.FirstChildElement(CNS_TAG_ROOT);
    mapTag = mapTag->FirstChildElement(CNS_TAG_LOG);
    XMLElement *element = doc.NewElement(CNS_TAG_RESULT);
    if (loglevel != CN_LP_LEVEL_NOPE_WORD) {
        element->InsertEndChild(doc.NewElement(CNS_TAG_SUM));
        element->InsertEndChild(doc.NewElement(CNS_TAG_PATH));
        element->InsertEndChild(doc.NewElement(CNS_TAG_LPLEVEL));
        element->InsertEndChild(doc.NewElement(CNS_TAG_HPLEVEL));
    }
    if (loglevel == CN_LP_LEVEL_FULL_WORD || loglevel == CN_LP_LEVEL_MEDIUM_WORD)
        element->InsertEndChild(doc.NewElement(CNS_TAG_LOWLEVEL));
    mapTag->InsertEndChild(element);
}

void XmlLogger::writeToLogMap(const Map &map, const std::list<Node> &path)
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD || loglevel == CN_LP_LEVEL_TINY_WORD)
        return;

    XMLElement *mapTag = doc.FirstChildElement(CNS_TAG_ROOT);
    mapTag = mapTag->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_PATH);

    int iterate = 0;
    bool inPath;
    std::string str;
    for (int i = 0; i < map.getMapHeight(); ++i) {
        XMLElement *element = doc.NewElement(CNS_TAG_ROW);
        element->SetAttribute(CNS_TAG_ATTR_NUM, iterate);

        for (int j = 0; j < map.getMapWidth(); ++j) {
            inPath = false;
            for(std::list<Node>::const_iterator it = path.begin(); it != path.end(); it++)
                if(it->i == i && it->j == j) {
                    inPath = true;
                    break;
                }
            if (!inPath)
                str += std::to_string(map.getValue(i,j));
            else
                str += CNS_OTHER_PATHSELECTION;
            str += CNS_OTHER_MATRIXSEPARATOR;
        }

        element->InsertEndChild(doc.NewText(str.c_str()));
        mapTag->InsertEndChild(element);
        str.clear();
        iterate++;
    }
}

void XmlLogger::writeToLogOpenClose(const std::vector<Node> &open, const std::set<Node, Node::CoordCompare> &close, int step)
{
    if (loglevel != CN_LP_LEVEL_FULL_WORD  && loglevel != CN_LP_LEVEL_MEDIUM_WORD)
        return;

    XMLElement *rootTag = doc.FirstChildElement(CNS_TAG_ROOT);
    XMLElement *lowLevelTag = rootTag->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_LOWLEVEL);
    XMLElement *stepTag = doc.NewElement(CNS_TAG_STEP);
    stepTag->SetAttribute(CNS_TAG_ATTR_NUM, step);

    XMLElement *openTag = doc.NewElement(CNS_TAG_OPEN);
    for (const auto &node : open) {
        openTag->InsertEndChild(createElement(node));
    }

    XMLElement *closeTag = doc.NewElement(CNS_TAG_CLOSE);
    for (const auto &node : close) {
        closeTag->InsertEndChild(createElement(node));
    }

    stepTag->InsertEndChild(openTag);
    stepTag->InsertEndChild(closeTag);
    lowLevelTag->InsertEndChild(stepTag);
}

void XmlLogger::writeToLogPath(const std::list<Node> &path)
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD || loglevel == CN_LP_LEVEL_TINY_WORD || path.empty())
        return;
    int iterate = 0;
    XMLElement *lplevel = doc.FirstChildElement(CNS_TAG_ROOT);
    lplevel = lplevel->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_LPLEVEL);

    for (std::list<Node>::const_iterator it = path.begin(); it != path.end(); it++) {
        XMLElement *element = doc.NewElement(CNS_TAG_POINT);
        element->SetAttribute(CNS_TAG_ATTR_X, it->j);
        element->SetAttribute(CNS_TAG_ATTR_Y, it->i);
        element->SetAttribute(CNS_TAG_ATTR_NUM, iterate);
        lplevel->InsertEndChild(element);
        iterate++;
    }
}

void XmlLogger::writeToLogHPpath(const std::list<Node> &hppath)
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD || loglevel == CN_LP_LEVEL_TINY_WORD || hppath.empty())
        return;
    int partnumber = 0;
    XMLElement *hplevel = doc.FirstChildElement(CNS_TAG_ROOT);
    hplevel = hplevel->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_HPLEVEL);
    std::list<Node>::const_iterator iter = hppath.begin();
    std::list<Node>::const_iterator it = hppath.begin();

    while (iter != --hppath.end()) {
        XMLElement *part = doc.NewElement(CNS_TAG_SECTION);
        part->SetAttribute(CNS_TAG_ATTR_NUM, partnumber);
        part->SetAttribute(CNS_TAG_ATTR_STX, it->j);
        part->SetAttribute(CNS_TAG_ATTR_STY, it->i);
        ++iter;
        part->SetAttribute(CNS_TAG_ATTR_FINX, iter->j);
        part->SetAttribute(CNS_TAG_ATTR_FINY, iter->i);
        part->SetAttribute(CNS_TAG_ATTR_LENGTH, iter->g - it->g);
        hplevel->LinkEndChild(part);
        ++it;
        ++partnumber;
    }
}

void XmlLogger::writeToLogSummary(unsigned int numberofsteps, unsigned int nodescreated, float length, double time, double cellSize)
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD)
        return;

    XMLElement *summary = doc.FirstChildElement(CNS_TAG_ROOT);
    summary = summary->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_SUM);
    XMLElement *element = summary->ToElement();
    element->SetAttribute(CNS_TAG_ATTR_NUMOFSTEPS, numberofsteps);
    element->SetAttribute(CNS_TAG_ATTR_NODESCREATED, nodescreated);
    element->SetAttribute(CNS_TAG_ATTR_LENGTH, length);
    element->SetAttribute(CNS_TAG_ATTR_LENGTH_SCALED, length*cellSize);
    element->SetAttribute(CNS_TAG_ATTR_TIME, std::to_string(time).c_str());
}

void XmlLogger::writeToLogNotFound()
{
    if (loglevel == CN_LP_LEVEL_NOPE_WORD)
        return;

    XMLElement *node = doc.FirstChildElement(CNS_TAG_ROOT)->FirstChildElement(CNS_TAG_LOG)->LastChildElement(CNS_TAG_RESULT)->FirstChildElement(CNS_TAG_PATH);
    node->InsertEndChild(doc.NewText("Path NOT found!"));
}

tinyxml2::XMLElement *XmlLogger::createElement(const Node &node) {
    XMLElement *tag = doc.NewElement(CNS_TAG_POINT);
    tag->SetAttribute(CNS_TAG_ATTR_X, node.j);
    tag->SetAttribute(CNS_TAG_ATTR_Y, node.i);
    tag->SetAttribute(CNS_TAG_ATTR_F, std::to_string(node.F()).c_str());
    tag->SetAttribute(CNS_TAG_ATTR_G, std::to_string(node.g).c_str());
    if (node.parent != std::make_pair(-1, -1)) {
        tag->SetAttribute(CNS_TAG_ATTR_PARX, node.parent.first);
        tag->SetAttribute(CNS_TAG_ATTR_PARY, node.parent.second);
    }
    return tag;
}
