const int DATA_MAX_SIZE = 4096;
const int MAX_NAME_LEN  = 128;
const int LISTEN_PORT   = 12345;
const int MAX_REQ_NUM   = 15;
const char FLAG_CH      = 127;   // a character which can't be in context.
const char ID_ADDR[16]  = "192.168.129.129";
// Error code
const int LISTEN_ERR    = -1;
const int BIND_ERR      = -2;
const int SOCKET_ERR    = -3;
const int CONNECT_ERR   = -4;