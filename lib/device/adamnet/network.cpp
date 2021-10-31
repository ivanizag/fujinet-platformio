#ifdef BUILD_ADAM

#include <cstring>
#include "adamnet/network.h"

adamNetwork::adamNetwork()
{
    Debug_printf("Network Start\n");
}

adamNetwork::~adamNetwork()
{
}

void adamNetwork::command_connect(uint16_t s)
{
    char hn[256];
    uint16_t p;

    s--; // remove command byte length.

    memset(&hn, 0, 256);

    // Get Port
    adamnet_recv_buffer((uint8_t *)&p, sizeof(uint16_t));

    s--;
    s--; // remove port length.

    // Get hostname
    adamnet_recv_buffer((uint8_t *)hn, s);

    // Get Checksum
    adamnet_recv();

    fnSystem.delay_microseconds(150);
    adamnet_send(0x9E); // Ack

    Debug_printf("Connecting to: %s port %u\n", hn, p);

    if (client.connected())
    {
        return;
    }

    if (alreadyDoingSomething == false)
    {
        // connect
        if (!client.connect(hn, p))
        {
        }
        else
        {
        }
    }
    Debug_println("Connect done.");
}

void adamNetwork::command_recv()
{
    if (client.available())
    {
        fnSystem.delay_microseconds(120);
        adamnet_send(0x9E);
        
        if (response_len == 0)
        {
            response_len = client.read(response, (client.available() > 1023 ? 1023 : client.available()));
            Debug_printf("recv: %s\n",response);
        }
    }
    else
    {
        fnSystem.delay_microseconds(120);
        adamnet_send(0xCE);
    }
}

void adamNetwork::command_send(uint16_t s)
{
    s--;
    adamnet_recv_buffer(response, s);
    adamnet_recv();

    fnSystem.delay_microseconds(100);
    adamnet_send(0x9E);
    
    client.write(response, s);
}

void adamNetwork::adamnet_control_status()
{
    uint8_t r[6] = {0x8E, 0x00, 0x04, 0x00, 0x00, 0x04};
    adamnet_send_buffer(r, 6);
}

void adamNetwork::adamnet_control_clr()
{
    adamnet_send(0xBE);
    adamnet_send_length(response_len);
    adamnet_send_buffer(response, response_len);
    adamnet_send(adamnet_checksum(response, response_len));
    memset(response,0,sizeof(response));
    response_len = 0;
}

void adamNetwork::adamnet_control_ready()
{
    if (isReady)
    {
        fnSystem.delay_microseconds(120);
        adamnet_send(0x9E); // ACK.
    }
}

void adamNetwork::adamnet_control_send()
{
    uint16_t s = adamnet_recv_length();
    uint8_t c = adamnet_recv();

    switch (c)
    {
    case 0xFF: // CONNECT TO HOST
        command_connect(s);
        break;
    case 0xFE: // SEND TO HOST
        command_send(s);
        break;
    }

    adamnet_send(0x9E);
}

void adamNetwork::adamnet_process(uint8_t b)
{
    unsigned char c = b >> 4;

    switch (c)
    {
    case MN_STATUS:
        adamnet_control_status();
        break;
    case MN_CLR:
        adamnet_control_clr();
        break;
    case MN_RECEIVE:
        command_recv();
        break;
    case MN_SEND:
        adamnet_control_send();
        break;
    case MN_READY:
        adamnet_control_ready();
        break;
    }

    fnUartSIO.flush_input();
    
}

#endif /* BUILD_ADAM */