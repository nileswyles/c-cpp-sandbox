#include "web/server_connection_etask.h"

#define RCV_SND_BUF_DEFAULTS 131072

// TODO: review openssl and socket documentation as well as throughly test this code because uncertainty is a thing.
void ServerConnectionETask::initialize() {
    // process_sockopts
    socklen_t byte_len = sizeof(uint8_t);
    socklen_t uint32_t_len = sizeof(uint32_t);
    socklen_t timeval_len = sizeof(struct timeval);

    uint32_t buf_size = RCV_SND_BUF_DEFAULTS;
    setsockopt(this->fd, SOL_SOCKET, SO_RCVBUF, &buf_size, uint32_t_len);
    setsockopt(this->fd, SOL_SOCKET, SO_SNDBUF, &buf_size, uint32_t_len);

    uint8_t keep_alive = 1;
    setsockopt(this->fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, byte_len);

    // close connection immediately
    uint8_t linger = 0;
    setsockopt(this->fd, SOL_SOCKET, SO_LINGER, &linger, byte_len);

    // ! IMPORTANT
    // For reads,
    // should be minimum value greater than max(SO_RCVBUF, READER_BUF_SIZE)/128kbps (aribitrary minimum connection speed)
    //  default RCVBUF == 131072, READER_BUF_SIZE == 8096, so ~1.024 seconds. Let's round to 2 seconds.
    struct timeval timeout = {
        .tv_sec = this->socket_timeout,
        .tv_usec = 0,
    };
    setsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(this->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);

    // read and print relevant options, ignore any error's reading... this is a nice to have...
    loggerExec(LOGGER_DEBUG_VERBOSE, 
        uint32_t rcv_buf_size = 0;
        getsockopt(this->fd, SOL_SOCKET, SO_RCVBUF, &rcv_buf_size, &uint32_t_len); // why pointer to len? also lame
        uint32_t snd_buf_size = 0;
        getsockopt(this->fd, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, &uint32_t_len);
        uint32_t rcv_lo_wat = 0;
        getsockopt(this->fd, SOL_SOCKET, SO_RCVLOWAT, &rcv_lo_wat, &uint32_t_len);
        uint32_t snd_lo_wat = 0;
        getsockopt(this->fd, SOL_SOCKET, SO_SNDLOWAT, &snd_lo_wat, &uint32_t_len);
        struct timeval rcv_timeout = {0};
        getsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
        struct timeval snd_timeout = {0};
        getsockopt(this->fd, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, &timeval_len);
        uint8_t ret_keep_alive = 1;
        getsockopt(this->fd, SOL_SOCKET, SO_KEEPALIVE, &ret_keep_alive, &byte_len);

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "SOCK OPTS: \n KEEP_ALIVE %u\n SO_RCVBUF %u\n SO_RCVLOWAT %u\n SO_SNDLOWAT %u\n SO_RCVTIMEO %lds %ldus\n SO_SNDBUF %u\n SO_SNDTIMEO %lds %ldus\n", 
            ret_keep_alive, rcv_buf_size, 
            rcv_lo_wat, snd_lo_wat,
            rcv_timeout.tv_sec, rcv_timeout.tv_usec, 
            snd_buf_size, snd_timeout.tv_sec, snd_timeout.tv_usec);
    );
}