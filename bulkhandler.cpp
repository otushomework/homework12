#include "bulkhandler.h"

BulkHandler::BulkHandler(std::size_t bulk)
    : m_parser(bulk)
{
#ifdef _DEBUG
    std::cout << "BulkHandler " << this << std::endl;
#endif

    m_parser.subscribe(std::bind(&Executor::execute, &m_executor, std::placeholders::_1));
    m_parser.subscribe(std::bind(&LogWriter::write, &m_logWritter, std::placeholders::_1));
}

BulkHandler::~BulkHandler()
{
#ifdef _DEBUG
    std::cout << "~BulkHandler " << this << std::endl;
#endif
}

void BulkHandler::receive(BulkHandler *handle, const char *data, std::size_t size)
{
    if (handle != this)
    {
        std::cout << "BulkHandler::receive: wrong handle " << handle << " != " << this << std::endl;
        return;
    }

#ifdef _DEBUG
    //std::cout << "BulkHandler::receive " << handle << " == " << this << " " << data << std::endl;
    std::cout << "BulkHandler::receive " << handle << " == " << this << std::endl;
#endif

    m_parser.receive(data, size);
}
