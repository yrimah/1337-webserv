#ifndef LOCATION_STORAGE_HPP
#define LOCATION_STORAGE_HPP

# include "conf_parser.hpp"
# include "server_storage.hpp"
# include <sys/stat.h>

size_t convertStrToInt(std::string str);

typedef std::vector<int> storage_int;
typedef std::vector<std::string> storage;

class Location_storage
{
    private:
        std::string loca_path;
		std::string loca_root;
		bool loca_autoindex;
		bool loca_upallow;
		std::string	loca_updir;
		std::string	loca_index;
		storage_int loca_allowed_methods;
		std::string	loca_redirect;
		std::string	loca_alias;
		storage	loca_cgi_path;
		storage	loca_cgi_extension;

    public:
        Location_storage();
        ~Location_storage();

        void init_location();
		bool check_maxsize(std::string maxsize);
		// setters //

		void setLocaAlias(std::string loca_alias);
		void setLocaAllowedMethods(storage loca_allowed_methods);
		void setLocaAutoindex(std::string loca_autoindex);
		void setLocaUpallow(std::string loca_upallow);
		void setLocaUPdir(std::string loca_updir);
		void setLocaCgipath(storage	loca_cgi_path);
		void setLocaCgiExtention(storage	loca_cgi_extension);
		void setLocaIndex(std::string loca_index);
		void setLocaPath(std::string loca_path);
		void setLocaRoot(std::string loca_root);
		void setLocaRedirect(std::string loca_redirect);
		
		// getters //

		const std::string &getLocaPath() const;
		const std::string &getLocaRoot() const;
		const storage_int &getLocaAllowedMethods() const;
		const bool &getLocaAutoindex() const;
		const std::string &getLocaIndex() const;
		const std::string &getLocaAlias() const;
		const storage &getLocaCgipath() const;
		const storage &getLocaCgiExtention() const;
		const bool &getLocaUpallow() const;
		const std::string &getLocaUpdir() const;
		const std::string &getLocaRedirect() const;

		class InvalidRootDir : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid root directory");
				}
		};

		class InvalidMethodException : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid method");
				}
		};

		class InvalidAutoindex : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid autoindex");
				}
		};

		class InvalidUpallow : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid up_allow");
				}
		};

		class InvalidUpDir : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid up_dir");
				}
		};

		class InvalidMaxSize : public std::exception
		{
			public:
				virtual const char * what() const throw()
				{
					return ("Invalid maxsize");
				}
		};
};

#endif