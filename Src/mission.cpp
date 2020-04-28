#include "mission.h"
#include <iostream>

Mission::Mission()
{
    logger = nullptr;
    fileName = nullptr;
}

Mission::Mission(const char *FileName)
{
    fileName = FileName;
    logger = nullptr;
}

Mission::~Mission()
{
    if (logger)
        delete logger;
}

bool Mission::getMap()
{
    return map.getMap(fileName);
}

bool Mission::getConfig()
{
    return config.getConfig(fileName);
}

bool Mission::createLog()
{
    if (logger != nullptr) delete logger;
    logger = new XmlLogger(config.LogParams[CN_LP_LEVEL]);
    return logger->getLog(fileName, config.LogParams);
}

void Mission::createEnvironmentOptions()
{
    options.searchtype = config.SearchParams[CN_SP_ST];
    options.cutcorners = config.SearchParams[CN_SP_CC];
    options.allowsqueeze = config.SearchParams[CN_SP_AS];
    options.allowdiagonal = config.SearchParams[CN_SP_AD];
    options.metrictype = config.SearchParams[CN_SP_MT];
    options.heuristicweight = config.SearchParams[CN_SP_HW];
    options.heuristicstep = config.SearchParams[CN_SP_HS];
    options.breakingties = config.SearchParams[CN_SP_BT];

}

void Mission::createSearch()
{
//might be helpful in case numerous algorithms are added
}

void Mission::startSearch()
{
    bool br = false;
    SearchResult sr;
    do {
        logger->writeToLogNewResult();
        std::tie(br, sr) = search.startSearch(logger, map, options);
        srs.push_back(sr);
        logger->writeToLogSummary(sr.numberofsteps, sr.nodescreated, sr.pathlength, sr.time, map.getCellSize());
        if (sr.pathfound) {
            logger->writeToLogPath(*sr.lppath);
            logger->writeToLogHPpath(*sr.hppath);
            logger->writeToLogMap(map, *sr.lppath);
        } else
            logger->writeToLogNotFound();
    } while (br);
}

void Mission::printSearchResultsToConsole()
{
    for (const auto &sr : srs) {
        std::cout << "Path ";
        if (!sr.pathfound)
            std::cout << "NOT ";
        std::cout << "found!" << std::endl;
        std::cout << "numberofsteps=" << sr.numberofsteps << std::endl;
        std::cout << "nodescreated=" << sr.nodescreated << std::endl;
        if (sr.pathfound) {
            std::cout << "pathlength=" << sr.pathlength << std::endl;
            std::cout << "pathlength_scaled=" << sr.pathlength * map.getCellSize() << std::endl;
        }
        std::cout << "time=" << sr.time << std::endl;
    }
}

void Mission::saveSearchResultsToLog()
{
    logger->saveLog();
}

SearchResult Mission::getSearchResult()
{
    return srs.back();
}

int Mission::logLevelIntValue() const {
    if (config.LogParams[CN_LP_LEVEL] == CN_LP_LEVEL_NOPE_WORD) {
        return 0;
    }
    if (config.LogParams[CN_LP_LEVEL] == CN_LP_LEVEL_TINY_WORD) {
        return 1;
    }
    if (config.LogParams[CN_LP_LEVEL] == CN_LP_LEVEL_SHORT_WORD) {
        return 2;
    }
    if (config.LogParams[CN_LP_LEVEL] == CN_LP_LEVEL_MEDIUM_WORD) {
        return 3;
    }
    if (config.LogParams[CN_LP_LEVEL] == CN_LP_LEVEL_FULL_WORD) {
        return 4;
    }
}

