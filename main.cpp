#include <iostream>

#include "async.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

//https://www.boost.org/doc/libs/1_36_0/doc/html/boost_asio/example/echo/async_tcp_echo_server.cpp

using boost::asio::ip::tcp;

class BulkSession {
public:
    BulkSession(boost::asio::io_service& io_service, std::size_t bulkSize)
        : m_socket(io_service)
    {
        m_asyncHandler = async::connect(bulkSize);
    }

    ~BulkSession()
    {
        async::disconnect(m_asyncHandler);
    }

    tcp::socket& socket()
    {
        return m_socket;
    }

    void start()
    {
        m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
                                 boost::bind(&BulkSession::handleRead, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

    void handleRead(const boost::system::error_code& error,
                    size_t bytes_transferred)
    {
        if (!error)
        {
            async::receive(m_asyncHandler, m_data, bytes_transferred);
            //todo
            //            boost::asio::async_write(m_socket,
            //                                     boost::asio::buffer(m_data, bytes_transferred),
            //                                     boost::bind(&session::handle_write, this,
            //                                                 boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }
private:
    tcp::socket m_socket;
    enum { max_length = 1024 };
    char m_data[max_length];
    async::handle_t m_asyncHandler;
};

class BulkServer
{
public:
    BulkServer(boost::asio::io_service& io_service, short port, std::size_t bulkSize)
        : m_io_service(io_service)
        , m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
        , m_bulkSize(bulkSize)
    {
        BulkSession* newSession = new BulkSession(m_io_service, m_bulkSize);
        m_acceptor.async_accept(newSession->socket(),
                                boost::bind(&BulkServer::handleAccept, this, newSession,
                                            boost::asio::placeholders::error));
    }

    void handleAccept(BulkSession* newSession,
                      const boost::system::error_code& error)
    {
        if (!error)
        {
            newSession->start();
            newSession = new BulkSession(m_io_service, m_bulkSize);
            m_acceptor.async_accept(newSession->socket(),
                                    boost::bind(&BulkServer::handleAccept, this, newSession,
                                                boost::asio::placeholders::error));
        }
        else
        {
            delete newSession;
        }
    }

private:
    boost::asio::io_service& m_io_service;
    tcp::acceptor m_acceptor;
    std::size_t m_bulkSize;
};

int main(int argc, char * argv[]) {
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: bulk_server <port> <bulkSize>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        using namespace std;
        BulkServer s(io_service, std::atoi(argv[1]), std::atoi(argv[2]));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
