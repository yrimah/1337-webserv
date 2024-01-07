#ifndef SERVER_STORAGE_HPP
# define SERVER_STORAGE_HPP

# include "location_storage.hpp"
# include "conf_reader.hpp"
//# include <netinet/in.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <map>
# include <sstream>
# include <vector> 

class Location_storage;
//////////////////////////////////////
std::string status(int); // for test //
//////////////////////////////////////

class Server_storage
{
	private:
		std::string _server_name;
		std::string _root;
		in_addr_t _host;
		uint16_t _port;
		size_t _client_max_body_size;
		std::string _index;
		bool _autoindex;
		std::map<int, std::string> _error_pages;
        int _listen_fd;
        struct sockaddr_in _server_address;

	public:
		int _reverse_host;

		std::vector<Location_storage> _locations;
		Server_storage();
		~Server_storage();
		Server_storage(const Server_storage &oServer_storage);
		Server_storage &operator=(const Server_storage & oServer_storage);


		void setServerName(std::string server_name);
		void setHost(std::string str);
		void setRoot(std::string root_path);
		void setFd(int);
		void setPort(std::string str);
		void setClientMaxBodySize(std::string str);
		void setErrorPages(std::vector<std::string> &str);
		void setIndex(std::string index);
		void setLocation(std::string nameLocation, std::vector<std::string> str);
		void setAutoindex(std::string autoindex);


		////////////////////////////////////////
		void set_sockadde(uint16_t Port, in_addr_t Host);
		struct sockaddr_in get_sockaddr() const;
		////////////////////////////////////////

		void initErrorPages(void);
		bool validateHost(std::string host) const;
		bool isValidErrorPages();
		int isValidLocation(Location_storage &location) const;

		// getters //
		const std::string &getServerName();
		const uint16_t &getPort();
		const in_addr_t &getHost();
		const size_t &getClientMaxBodySize();
		const std::vector<Location_storage> &getLocations();
		const std::string &getRoot();
		const std::map<int, std::string> &getErrorPages();
		const std::string &getIndex();
		const bool &getAutoindex();
		const std::string &getPathErrorPage(int code);
		const std::vector<Location_storage>::iterator getLocationKey(std::string key);
        int getFd();

		static void removeDelim(std::string &parametr);
		bool isLocaDup() const;

		public:
		class ExceptionMsg : public std::exception
		{
			private:
				std::string status;
			public:
				ExceptionMsg(std::string error) throw()
				{
					status = "Error in server configuration " + error;
				}
				virtual const char* what() const throw()
				{
					return (status.c_str());
				}
				virtual ~ExceptionMsg() throw()
				{
				}
		};

		void	setupServer();

};

#endif