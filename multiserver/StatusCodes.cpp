#include "../incs/parse_config/StatusCodes.hpp"

StatusCodes::StatusCodes()
{
    OK_200 = "HTTP/1.1 200 ok\r\n";
    NotFond_404 = "HTTP/1.1 404 not Found\r\n";
    Forbidden_403 = "HTTP/1.1 403 forbidden\r\n";
    NoContent_204 = "HTTP/1.1 204 No Content\r\n\r\n";
    Method_NAllowed_405 = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    Moved_Permanently_301 = "HTTP/1.1 301 Moved Permanently\r\n";
    Bad_Request_400 = "HTTP/1.1 400 Bad Request\r\n";
    Length_Required_411 = "HTTP/1.1 411 Length Required\r\n";
    Content_Too_Large_413 = "HTTP/1.1 413 Content Too Large\r\n";
    URI_Too_Long_414 = "HTTP/1.1 414 URI Too Long\r\n";
    Unsupported_Media_Type_415 = "HTTP/1.1 415 Unsupported Media Type\r\n";
    Internal_server_Error_500 = "HTTP/1.1 500 internal server error\r\n";
    Not_Implemented_501 = "HTTP/1.1 501 not implemented\r\n";
    Bad_Gateaway_502 = "HTTP/1.1 502 bad gateaway\r\n";
    Gateaway_Timeout_504 = "HTTP/1.1 504 gateway timeout\r\n";
}

StatusCodes::~StatusCodes() {}
