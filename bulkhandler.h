#ifndef BULKHANDLER_H
#define BULKHANDLER_H

#include <cstddef>
#include <iostream>
#include "config.h"
#include <list>
#include <sstream>
#include <functional>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <mutex>
#include <condition_variable>

class Parser
{
    enum class ParsingState
    {
        TopLevel = 0,
        InBlock = 1
    };

public:
    Parser (int bulkSize)
        : m_bulkSize(bulkSize) { }
    ~Parser()
    {
        publish();
    }

    void receive(const char *data, std::size_t size)
    {
        std::string fullData(data, size);
        std::istringstream fdStream(fullData);

        while (std::getline(fdStream, m_line))
        {
            if (m_line.size() == 0)
                continue;

            switch (m_state)
            {
            case ParsingState::TopLevel:
            {
                if (m_line != "{")
                {
                    m_commands.push_back(m_line);
                    if (m_commands.size() == m_bulkSize)
                        publish();
                    break;
                }
                else
                {
                    m_depthCounter++;
                    publish();
                    m_state = ParsingState::InBlock;
                    break;
                }
            }
            case ParsingState::InBlock:
            {
                if (m_line != "}")
                {
                    if (m_line == "{")
                        m_depthCounter++;
                    else
                        m_commands.push_back(m_line);
                }
                else
                {
                    m_depthCounter--;
                    if (m_depthCounter == 0)
                    {
                        publish();
                        m_state = ParsingState::TopLevel;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }

    void subscribe(const std::function<void(std::list<std::string>)>& callback)
    {
        m_subscribers.push_back(callback);
    }

    void publish()
    {
        if (m_commands.size() == 0)
            return;

        for (const auto& subscriber : m_subscribers)
        {
            subscriber(m_commands);
        }
        m_commands.clear();
    }

private:
    std::list<std::function<void(std::list<std::string>)> > m_subscribers;
    int m_bulkSize;

    //store data between launches
    ParsingState m_state = ParsingState::TopLevel;
    std::list<std::string> m_commands;
    int m_depthCounter = 0;
    std::string m_line;
};

class Executor
{
public:
    void execute(std::list<std::string> commands)
    {
        std::cout << "bulk:";
        for (auto command : commands)
            std::cout << command << " ";
        std::cout << std::endl;
    }
};

class LogWriter
{
public:
    LogWriter() { }

    ~LogWriter() { }

    void write(std::list<std::string> commands)
    {
        static int conflictResolverCounter = 0;
        static std::mutex conflictMutex;

        std::ofstream logFile;

        std::time_t result = std::time(nullptr);
        char buff[FILENAME_MAX];
        getcwd(buff, FILENAME_MAX );
        std::string current_working_dir(buff);
        {
            std::lock_guard<std::mutex> lk(conflictMutex);
            logFile.open (std::string(current_working_dir + "/bulk" + std::to_string(result) + "_" + std::to_string(conflictResolverCounter) + ".log"));
            std::cout << std::string(current_working_dir + "/bulk" + std::to_string(result) + "_" + std::to_string(conflictResolverCounter) + ".log") << std::endl;
            conflictResolverCounter++;
        }
        logFile << "bulk:";
        for (auto command : commands)
            logFile << command << " ";
        logFile << std::endl;

        logFile.close();
    }
};

class BulkHandler
{
public:
    BulkHandler(std::size_t bulk);
    ~BulkHandler();
    void receive(BulkHandler *handle, const char *data, std::size_t size);

private:
    Parser m_parser;
    Executor m_executor;
    LogWriter m_logWritter;
};

#endif // BULKHANDLER_H
