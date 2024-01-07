#pragma once

#include <iostream>

class StatusCodes
{
    public :
        std::string OK_200;
        std::string NotFond_404;
        std::string Forbidden_403;
        std::string NoContent_204;
        std::string Method_NAllowed_405;
        std::string Moved_Permanently_301;
        std::string Bad_Request_400;
        std::string Length_Required_411;
        std::string Content_Too_Large_413;
        std::string URI_Too_Long_414;
        std::string Unsupported_Media_Type_415;
        std::string Internal_server_Error_500;
        std::string Not_Implemented_501;
        std::string Bad_Gateaway_502;
        std::string Gateaway_Timeout_504;
    public :
        StatusCodes();
        ~StatusCodes();
};

