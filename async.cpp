#include "async.h"
#include "bulkhandler.h"
#include <vector>

namespace async
{

std::vector<BulkHandler *> g_handlers;

handle_t connect(std::size_t bulk)
{
    BulkHandler *bh = new BulkHandler(bulk);
    g_handlers.push_back(bh);
    return static_cast<handle_t>(bh);
}

void receive(handle_t handle, const char *data, std::size_t size)
{
    if ( std::find(g_handlers.begin(), g_handlers.end(), handle) != g_handlers.end() )
    {
        BulkHandler* castedHandler = static_cast<BulkHandler *>(handle); //howto check is cast wrong?

        castedHandler->receive(castedHandler, data, size);
    }
    else
    {
        std::cout << "Receive: handle not found!";
    }
}

void disconnect(handle_t handle)
{
    if ( std::find(g_handlers.begin(), g_handlers.end(), handle) != g_handlers.end() )
    {
        BulkHandler* castedHandler = static_cast<BulkHandler *>(handle); //howto check is cast wrong?

        delete castedHandler;
    }
    else
    {
        std::cout << "Disconnect: handle not found!";
    }
}

}
