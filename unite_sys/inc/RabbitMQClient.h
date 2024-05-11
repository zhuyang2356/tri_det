/*
 * RabbitMQClient.h
 *
 *  Created on: Feb 27, 2023
 *      Author: sne
 */

#ifndef INC_RABBITMQCLIENT_H_
#define INC_RABBITMQCLIENT_H_



#include <ev.h>
#include <amqp/amqpcpp.h>
// #include <amqpcpp/libev.h>
#include <amqp/amqpcpp/libev.h>

 /**
 *  Custom handler
 */
class RabbitMQClientHandler : public AMQP::LibEvHandler
{
private:
    /**
     *  Method that is called when a connection error occurs
     *  @param  connection
     *  @param  message
     */
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)【RabbitMQClientHandler】error:%s \n"),message));
    }

    /**
     *  Method that is called when the TCP connection ends up in a connected state
     *  @param  connection  The TCP connection
     */
    virtual void onConnected(AMQP::TcpConnection *connection) override
    {
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)【RabbitMQClientHandler】connected! \n")));
    }

    /**
     *  Method that is called when the TCP connection ends up in a ready
     *  @param  connection  The TCP connection
     */
    virtual void onReady(AMQP::TcpConnection *connection) override
    {
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)【RabbitMQClientHandler】ready! \n")));
    }

    /**
     *  Method that is called when the TCP connection is closed
     *  @param  connection  The TCP connection
     */
    virtual void onClosed(AMQP::TcpConnection *connection) override
    {
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)【RabbitMQClientHandler】closed! \n")));
    }

    /**
     *  Method that is called when the TCP connection is detached
     *  @param  connection  The TCP connection
     */
    virtual void onDetached(AMQP::TcpConnection *connection) override
    {
        ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%D)【RabbitMQClientHandler】detached! \n")));
    }

public:
    /**
     *  Constructor
     *  @param  ev_loop
     */
    RabbitMQClientHandler(struct ev_loop *loop) : AMQP::LibEvHandler(loop) {}

    /**
     *  Destructor
     */
    virtual ~RabbitMQClientHandler() = default;
};


#endif /* INC_RABBITMQCLIENT_H_ */
