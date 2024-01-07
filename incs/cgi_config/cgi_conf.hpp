#ifndef CGI_CONF_HPP
# define CGI_CONF_HPP

# include "../parse_config/location_storage.hpp"
# include "../parse_config/Request.hpp"
# include <vector>
# include <map>
# include <iostream>
# include <cstdlib>
# include <string.h>
# include <cstring>
# include <sys/types.h>
# include <sys/wait.h>

typedef std::map<std::string, std::string> map_env;
typedef std::map<std::string, std::string>::const_iterator map_it;
typedef std::vector<Location_storage>::const_iterator Loca_it;

class Cgi_conf
{
    private:
        map_env cgi_env;
        std::string path;
        char    ** _env;
        char    **_argv;
        int     status;

    public:
        Cgi_conf();
        ~Cgi_conf();

        pid_t   cgi_id;
        int     _in[2];
        int     _out[2];
        std::string final;

        void initialize_cgi_env(Request requestObj, Loca_it _it);

        void cgi_executer(Request &oRequest, int &error);
        void clear();

        void setPath(std::string path);
        int &getStatus(void);

        public:
            class CgiException : public std::exception
		{
			private:
				std::string status;
			public:
				CgiException(std::string error) throw()
				{
					status = "Error in server Cgi : " + error;
				}
				virtual const char* what() const throw()
				{
					return (status.c_str());
				}
				virtual ~CgiException() throw()
				{
				}
		};
};

#endif