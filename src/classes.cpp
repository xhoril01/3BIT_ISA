/**
 * @file classes.cpp
 * @author Denis HORIL (xhoril01@stud.fit.vutbr.cz)
 * @brief ISA Projekt - Pomocny subor k hlavnemu suboru 'feedreader.cpp'
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <getopt.h>
#include <fstream>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <libxml/parser.h>

using namespace std;

// Errors handling
#define ERR_STAT(_m) { \
            fprintf(stderr,(_m "\n")); \
        }
#define ERR_STAT_ARG(_m, ...) { \
            fprintf(stderr,(_m "\n"), ##__VA_ARGS__); \
        } 

// Destructors
#define IM_FREEEEE { \
            if(bio) BIO_free_all(bio); \
            if(ctx) SSL_CTX_free(ctx); \
        }


// Use(ful|less) macros
// #define RDF_FEED 1
#define RSS_FEED 2
#define ATOM_FEED 3
#define UNKNOWN_FEED -1

using namespace std;

// Global variables for better interruption handling
BIO *bio;
SSL_CTX *ctx;
SSL *ssl;

/**
 * @brief Parsed URL
 * 
 */
struct parsedURL
{
    std::string scheme;
    std::string host;
    std::string port;
    std::string path;
    std::stringstream authority;
};

/**
 * @brief Feed information
 * 
 */
struct Feed
{
    std::vector<std::string> URLList;
    std::string path;
};

/**
 * @brief Author information
 * 
 */
struct Author
{
    std::string name;
    std::string email;
};

/**
 * @brief Content from XML file given by response
 * 
 */
struct XMLContent
{
    std::string title;
    std::vector<std::string> aURL;
    std::vector<Author> authors;
    std::string time;
};

/**
 * @brief Checking URL scheme
 * 
 * @param scheme Scheme from URL
 * @param str HTTP or HTTPS
 * @return true 
 * @return false 
 */
bool schemeCheck(std::string scheme, std::string str)
{
    if(scheme.compare(str) == 0) return true;
    else return false;
}

/**
 * @brief Parsing URL
 * 
 * @param url Actual URL
 * @param parsedURL Structure where parsed URL will be stored
 * @return true if successful
 * @return false otherwise
 */
bool checkURL(std::string url, parsedURL *parsedURL)
{
    std::regex regexExpr ("^(https?)://([^:?#/]+)(:([0-9]+))?(.*)$");
    std::smatch matches;

    if(!std::regex_match(url, matches, regexExpr)) 
    {
        ERR_STAT_ARG("Incorrect URL : '%s'\nOnly 'http' and 'https' schemes are allowed",url.c_str());
        return false;
    }

    parsedURL->scheme = matches[1];
    parsedURL->host = matches[2];
    parsedURL->path = matches[5];
    
    if(matches[3].str().empty())
    {
        if(schemeCheck(matches[1], "http")) parsedURL->port = "80";
        else if(schemeCheck(matches[1], "https")) parsedURL->port = "443";
    }
    else parsedURL->port = matches[4];

    parsedURL->authority << parsedURL->host << ':' << parsedURL->port;

    return true;
}


class Args
{
    private:
        std::stringstream url;
        std::stringstream feedFile;
        std::stringstream certFile;
        std::stringstream certAddr;
        bool timeFlag = false;
        bool authorFlag = false;
        bool assocURLFlag = false;
        bool helpFlag = false;

    public:
        /**
         * @brief Processing arguments from command line
         * 
         * @param argc 
         * @param argv 
         * @return int - 0 if successful, -1 otherwise
         */
        int argsProcesser(int argc, char** argv)
        {
            opterr = 0;
            const char* const paramShort = "f:c:C:Tuah";
            const option paramLong[]=
            {
                {"help", no_argument, nullptr, 'h'},
                {nullptr, no_argument, nullptr, 0}
            };

            
            int arg;

            // Getting arguments from cmd line
            while((arg = getopt_long(argc,argv,paramShort, paramLong, nullptr)) != -1)
            {
                switch(arg)
                {
                    case 'f':
                            feedFile << optarg;
                            break;
                    
                    case 'c':
                            certFile << optarg;
                            break;

                    case 'C':
                            certAddr << optarg;
                            break;
                    case 'T':
                            timeFlag = true;
                            break;

                    case 'u':
                            assocURLFlag = true;
                            break;

                    case 'a':
                            authorFlag = true;
                            break;
                    
                    case 'h':
                            helpFlag = true;
                            printHelp();
                            break;

                    case '?':
                            if(optopt == 'f' || optopt == 'c' || optopt == 'C')
                            {
                                ERR_STAT("-f, -c and -C flags require argument.");
                                return -1;
                            }

                            else if(isprint(optopt))
                            {
                                ERR_STAT_ARG("Unknown flag: -%c.",optopt);
                                return -1;
                            }
                                
                            else
                            {
                                ERR_STAT_ARG("Unknown character: %02x.",optopt);
                                return -1;
                            } 
                            
                    default: 
                            ERR_STAT("Argument processing failed.");
                            return -1;           
                }
            }

            // Magic
            if(optind < argc && (argc-optind) <= 1)
            {
                url << argv[optind];
            }
            else if((argc-optind) > 1)
            {
                if(!helpFlag)
                {
                    ERR_STAT("None or unknown parameter was given");
                    return -1;
                }
            }

            return 0;
        }

        /**
         * @brief Returns URL
         * 
         * @return std::string 
         */
        std::string getURL()
        {
            return url.str();
        }

        /**
         * @brief Get the path to <feedfile>
         * 
         * @return std::string 
         */
        std::string getFeedFile()
        {
            return feedFile.str();
        }

        /**
         * @brief Get the path <certfile>
         * 
         * @return std::string 
         */
        std::string getCertFile()
        {
            return certFile.str();
        }

        /**
         * @brief Get the path to <certaddr>
         * 
         * @return std::string 
         */
        std::string getCertAddr()
        {
            return certAddr.str();
        }

        /**
         * @brief Was -T defined?
         * 
         * @return true if -T was defined
         * @return false otherwise
         */
        bool isTime()
        {
            return timeFlag;
        }

        /**
         * @brief Was -a defined?
         * 
         * @return true if -a was defined
         * @return false otherwise
         */
        bool isAuthor()
        {
            return authorFlag;
        }

        /**
         * @brief Was -u defined?
         * 
         * @return true if -u was defined
         * @return false otherwise
         */
        bool isAssocURL()
        {
            return assocURLFlag;
        }

        /**
         * @brief Was -h or --help defined?
         * 
         * @return true if -h or --help was defined
         * @return false otherwise
         */
        bool isHelp()
        {
            return helpFlag;
        }

        /**
         * @brief Required arguments check
         * 
         * @return int 0 if it is successful, -1 otherwise
         */
        int reqArgsCheck()
        {
            if(url.str().empty() && feedFile.str().empty())
            {
                ERR_STAT("One of 'url' or '-f <feedfile>' argument is required.");
                return -1;
            }
            else if (!url.str().empty() && !feedFile.str().empty())
            {
                ERR_STAT("One of 'url' or '-f <feedfile>' argument is required, but NOT both.");
                return -1;
            }
            else return 0;
        }

        /**
         * @brief Help statement
         * 
         */
        void printHelp()
        {
            cout << "\n=================================================== HELP ==================================================" << endl;
            cout << "|                                                                                                          |" << endl;
            cout << "|   USAGE: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-u] [-a] [-h | --help]   |" << endl;
            cout << "|                 URL - resource URL                                                                       |" << endl;
            cout << "|       -f <feedfile> - path to feedfile                                                                   |" << endl;
            cout << "|       -c <certfile> - path to file with certificates                                                     |" << endl;
            cout << "|       -C <certaddr> - path to directory with certficates                                                 |" << endl;
            cout << "|                  -T - display last edit time or creation time (if available)                             |" << endl;
            cout << "|                  -u - display associated URLs (if available)                                             |" << endl;
            cout << "|                  -a - display information about author (if available)                                    |" << endl;
            cout << "|                                                                                                          |" << endl;
            cout << "|  * One of arguments <URL> or <-f <feedfile>> must be defined, but NOT both.                              |" << endl;
            cout << "|  * If <-c <certfile>> or <-C <certaddr>> are not defined, 'SSL_CTX_set_default_verify_paths()' function  |" << endl;
            cout << "|    will be used.                                                                                         |" << endl;
            cout << "|  * While HTTP communication with server, redirecting is NOT implemented                                  |" << endl;
            cout << "|  * In URL can be optionally specificated 'port number'                                                   |" << endl;
            cout << "|                                                                                                          |" << endl;
            cout << "============================================================================================================\n" <<endl;
            return;
        }

};

class Process
{
    private:
        Feed feed;

    public:
        /**
         * @brief Set the path of feedfile
         * 
         * @param path 
         */
        void setPath(std::string path)
        {
            feed.path = path;
        }

        /**
         * @brief Creating list of URLs from feedfile
         * 
         * @return int - 0 if successful, -1 otherwise
         */
        int feedfile2List()
        {
            ifstream file(feed.path);
            std::string data;

            try
            {
                if(!file.is_open()) throw std::exception();
            
                while(getline(file,data))
                {
                    if(!data.empty() && data.front() != '#')
                    {
                        feed.URLList.push_back(data);
                    }
                }

                file.close();
            }
            catch(std::exception &e)
            {
                ERR_STAT_ARG("Cannot open file: %s",feed.path.c_str());
                return -1;
            }

            return 0;
        }

        /**
         * @brief Connecting every URL from feedfile
         * 
         * @param args Class with arguments
         */
        void loopConnect(Args *args)
        {
            for(std::string url : feed.URLList)
            {
                parsedURL myURL;
                if(!checkURL(url, &myURL)) continue;
                if(connect(&myURL, args, url) == -1)
                {                 
                    continue;
                }
            }

            return;
        }

        /**
         * @brief Conecting to given URL
         * 
         * @param pURL URL parsed to scheme, authority, etc.
         * @param args Class with arguments
         * @param url actual URL
         * @return int - 0 if succesful, -1 otherwise
         */
        int connect(parsedURL *pURL, Args *args, std::string url)
        {
            SSL_load_error_strings();
            ERR_load_BIO_strings();
            OpenSSL_add_all_algorithms();
            SSL_library_init();

            // Unsecure connection
            if(schemeCheck(pURL->scheme, "http"))
            {
                bio = BIO_new_connect(pURL->authority.str().c_str());
            }
            else
            {
                ctx = SSL_CTX_new(SSLv23_client_method());
                int verify = 0;

                if(args->getCertFile().empty() &&
                   args->getCertAddr().empty())
                    {
                        verify = SSL_CTX_set_default_verify_paths(ctx);
                    }
                
                else if(!args->getCertFile().empty() &&
                        !args->getCertAddr().empty())
                    {
                        verify = SSL_CTX_load_verify_locations(ctx, args->getCertFile().c_str(), args->getCertAddr().c_str());
                    }
                
                else if(!args->getCertFile().empty())
                    {
                        verify = SSL_CTX_load_verify_locations(ctx,args->getCertFile().c_str(),nullptr);
                    }
                else 
                    {
                        verify = SSL_CTX_load_verify_locations(ctx,nullptr, args->getCertAddr().c_str());
                    }

                if(verify == 0)
                {
                    ERR_STAT_ARG("Bio error: %s on url '%s'",ERR_error_string(ERR_get_error(),nullptr),url.c_str());
                    IM_FREEEEE;
                    return -1;
                }

                bio = BIO_new_ssl_connect(ctx);
            }

            if(bio == nullptr) 
            {
                ERR_STAT_ARG("Bio error: %s, connection failed on url '%s'", ERR_error_string(ERR_get_error(), nullptr), url.c_str());
                IM_FREEEEE;
                return -1;
            }
            
            if(schemeCheck(pURL->scheme, "https"))
            {
                BIO_get_ssl(bio, &ssl);
                SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
                BIO_set_conn_hostname(bio, pURL->authority.str().c_str());
                SSL_set_tlsext_host_name(ssl, pURL->host.c_str());
            }   

            if(BIO_do_connect(bio) <= 0)
            {
                ERR_STAT_ARG("Bio error: %s, connection failed on url '%s'", ERR_error_string(ERR_get_error(), nullptr), url.c_str());
                IM_FREEEEE;
                return -1;
            }

            if(schemeCheck(pURL->scheme, "https") && (SSL_get_verify_result(ssl) != X509_V_OK))
            {
                ERR_STAT_ARG("Bio error: %s, certificates verification failed on url '%s'", ERR_error_string(ERR_get_error(),nullptr), url.c_str());
                IM_FREEEEE;
                return -1;
            }

            std::stringstream request;
            request << "GET " << pURL->path << " HTTP/1.0\r\n";
            request << "Host: " << pURL->authority.str() << "\r\n";
            request << "Accept: */*\r\n";
            request << "Connection: Close\r\n";
            request << "\r\n";

            bool writeFlag = false;
            if(BIO_write(bio, request.str().c_str(), request.str().size()) <= 0)
            {
                while(BIO_should_retry(bio))
                {
                    if(BIO_write(bio, request.str().c_str(), request.str().size()) > 0)
                    {
                        writeFlag = true;
                        break;
                    }
                }
            }
            else writeFlag = true;

            if(!writeFlag)
            {
                ERR_STAT_ARG("Bio write error: %s, on url '%s'", ERR_error_string(ERR_get_error(),nullptr), url.c_str());
                IM_FREEEEE;
                return -1;
            }

            char buffer[2048] = {'\0'};
            std::stringstream response;
            bool readFlag = false;
            int bytes = BIO_read(bio, buffer, 2048-1);
            
            while(bytes != 0)
            {
                while(BIO_should_retry(bio))
                {
                    if(bytes >= 0)
                    {
                        if(bytes > 0)
                        {
                            buffer[bytes] = '\0';
                            response << buffer;
                        }
                        readFlag = true;
                        bytes = BIO_read(bio, buffer, 2048-1);
                        break;
                    }
                }

                if(bytes >= 0)
                {
                    if(bytes > 0)
                    {
                        buffer[bytes] = '\0';
                        response << buffer;
                    }
                    readFlag = true;
                    bytes = BIO_read(bio, buffer, 2048-1);
                }

                if(!readFlag)
                {
                    ERR_STAT_ARG("Bio read error: %s, on url '%s'", ERR_error_string(ERR_get_error(),nullptr), url.c_str());
                    IM_FREEEEE;
                    return -1;
                }
            }

            std::string XMLFile = responseProcess(response.str());

            if(XMLFile.empty())
            { 
                ERR_STAT_ARG("Invalid HTTP response on url '%s'", url.c_str());
                IM_FREEEEE;
                return -1;
            }            

            if(parseXML(XMLFile, args, url) == -1)
            {
                IM_FREEEEE;
                return -1;
            }

            IM_FREEEEE;
            return 0;
        }

        /**
         * @brief Processing HTTP response
         * 
         * @param response HTTP response
         * @return std::string - returns XML file parsed from response,
         *                       NULL if response is invalid
         */
        std::string responseProcess(std::string response)
        {
            std::string body;
            std::string httpCode;
            httpCode.assign(response.begin(), response.begin()+response.find("\r\n"));
            std::regex reg("^(HTTP/[0-9]\\.[0-9]\\s2[0-9][0-9]\\s[a-zA-Z]+)$");

            if(!regex_match(httpCode,reg))
            {
                return body;
            }
            
            int contPosStart = response.find("<");
            int contPosEnd = response.find_last_of(">");
            body.assign(response.begin()+contPosStart, response.begin()+contPosEnd+1);
            
            return body;
        }

        /**
         * @brief Parsing given XML file from HTTP response
         * 
         * @param file XML file
         * @param args Class with arguments
         * @param url Actual URL
         * @return int 0 of successful, -1 otherwise
         */
        int parseXML(std::string file, Args *args, std::string url)
        {
            LIBXML_TEST_VERSION

            xmlDocPtr doc = nullptr;
            xmlNodePtr rootNode = nullptr;

            if((doc = xmlParseDoc((const xmlChar *)file.c_str())) == nullptr) 
            {
                ERR_STAT_ARG("Parsing given XML file on url '%s' failed", url.c_str());
                xmlCleanupParser();
                return -1;
            }

            if((rootNode = xmlDocGetRootElement(doc)) == nullptr)
            {
                ERR_STAT_ARG("Missing root element in XML file on url '%s'", url.c_str());
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return -1;
            }

            std::vector<XMLContent> content2print;
            std::string feedTitle;

            switch(getFeedType(rootNode))
            {
                /*
                case RDF_FEED:
                        content2print = rdfFeed(rootNode, &feedTitle);
                        break;
                */

                case RSS_FEED:
                        content2print = rssFeed(rootNode, &feedTitle);
                        break;

                case ATOM_FEED:
                        content2print = atomFeed(rootNode, &feedTitle);
                        break;

                default: break;
            }

            if(content2print.empty())
            {
                ERR_STAT_ARG("Unknown feed type on url '%s'", url.c_str());
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return -1;
            }

            if(feedTitle.empty())
            {
                ERR_STAT_ARG("Missing title of the feed on url '%s'", url.c_str());
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return -1;
            }

            printInfo(content2print, feedTitle, args);
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return 0;
        }

        /**
         * @brief Get the Feed type
         * 
         * @param rootNode Root element
         * @return int 1,2 or 3 depending on type of feed, -1 otherwise
         */
        int getFeedType(xmlNodePtr rootNode)
        {
            // if(!xmlStrcmp(rootNode->name, (const xmlChar*)"rdf")) return RDF_FEED;

            if(!xmlStrcmp(rootNode->name, (const xmlChar*)"rss")) return RSS_FEED;

            if(!xmlStrcmp(rootNode->name, (const xmlChar*)"feed")) return ATOM_FEED;

            return UNKNOWN_FEED;
        }

        /**
         * @brief Parsing RSS 1.0 a.k.a. RDF feed
         * 
         * @param rootNode Root element of parsed document
         * @param title Pointer to main title of the feed
         * @return std::vector<XMLContent> List of parsed items
         */

        /*
        std::vector<XMLContent> rdfFeed(xmlNodePtr rootNode, std::string *title)
        {
            XMLContent write;
            Author author;
            std::vector<XMLContent> writeList;
            xmlChar *content;
            
            for(xmlNodePtr curNode = rootNode->xmlChildrenNode; curNode; curNode = curNode->next)
            {
                if(!xmlStrcmp(curNode->name, (const xmlChar*)"channel"))
                {
                    for(xmlNodePtr childNode = curNode->xmlChildrenNode; childNode; childNode = childNode->next)
                    {
                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"title"))
                        {
                            content = xmlNodeGetContent(childNode);
                            *title = (char*)content;
                            xmlFree(content);       
                            break;
                        }
                    }
                }

                if(!xmlStrcmp(curNode->name, (const xmlChar*)"item"))
                {
                    for(xmlNodePtr childNode = curNode->xmlChildrenNode; childNode; childNode = childNode->next)
                    {
                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"title"))
                        {
                            content = xmlNodeGetContent(childNode);
                            write.title = (char*)content;
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"link"))
                        {
                            content = xmlNodeGetContent(childNode);
                            write.aURL.push_back((char*)content);
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"dc:creator"))
                        {
                            content = xmlNodeGetContent(childNode);
                            author.name = (char*)content;
                            write.authors.push_back(author);
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"dc:date"))
                        {
                            content = xmlNodeGetContent(childNode);
                            write.time = (char*)content;
                            xmlFree(content);
                        }
                    }

                    writeList.push_back(write);
                    write.aURL.clear();
                    write.authors.clear();
                    write.time.clear();
                    write.title.clear();
                }
            }

            xmlCleanupParser();
            return writeList;
        }
        */

        /**
         * @brief Parsing RSS 2.0 feed
         * 
         * @param rootNode Root element of parsed document
         * @param title Pointer to main title of the feed
         * @return std::vector<XMLContent> List of parsed items
         */
        std::vector<XMLContent> rssFeed(xmlNodePtr rootNode, std::string *title)
        {
            XMLContent write;
            Author author;
            std::vector<XMLContent> writeList;
            xmlChar *content;
            
            for(xmlNodePtr curNode = rootNode->xmlChildrenNode; curNode; curNode = curNode->next)
            {   
                if(!xmlStrcmp(curNode->name, (const xmlChar*)"channel"))
                {
                    for(xmlNodePtr childNode = curNode->xmlChildrenNode; childNode; childNode = childNode->next)
                    {
                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"title"))
                        {
                            content = xmlNodeGetContent(childNode);
                            *title = (char*)content;
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"item"))
                        {
                            for(xmlNodePtr itemNode = childNode->xmlChildrenNode; itemNode; itemNode = itemNode->next)
                            {
                                if(!xmlStrcmp(itemNode->name, (const xmlChar*)"title"))
                                {
                                    content = xmlNodeGetContent(itemNode); 
                                    write.title = (char*)content;
                                    xmlFree(content);
                                }

                                if(!xmlStrcmp(itemNode->name, (const xmlChar*)"link"))
                                {
                                    content = xmlNodeGetContent(itemNode); 
                                    write.aURL.push_back((char*)content);
                                    xmlFree(content);              
                                }

                                if(!xmlStrcmp(itemNode->name, (const xmlChar*)"author"))
                                {
                                    content = xmlNodeGetContent(itemNode);
                                    author.name = (char*)content;
                                    write.authors.push_back(author);
                                    xmlFree(content);
               
                                }

                                if(!xmlStrcmp(itemNode->name, (const xmlChar*)"pubDate"))
                                {
                                    content = xmlNodeGetContent(itemNode);
                                    write.time = (char*)content;
                                    xmlFree(content);
                                }
                            }

                            writeList.push_back(write);
                            write.aURL.clear();
                            write.authors.clear();
                            write.time.clear();
                            write.title.clear();
                        }
                    }
                }
            }

            xmlCleanupParser();
            return writeList;
        }

        /**
         * @brief Parsing Atom feed
         * 
         * @param rootNode Root element of parsed document
         * @param title Pointer to main title of the feed
         * @return std::vector<XMLContent> List of parsed items
         */
        std::vector<XMLContent> atomFeed(xmlNodePtr rootNode, std::string *title)
        {
            XMLContent write;
            Author author;
            std::vector<XMLContent> writeList;
            xmlChar *content;

            for(xmlNodePtr curNode = rootNode->xmlChildrenNode; curNode; curNode = curNode->next)
            {
                if(!xmlStrcmp(curNode->name, (const xmlChar*)"title"))
                {
                    content = xmlNodeGetContent(curNode);
                    *title = (char*)content;
                    xmlFree(content);
                }

                if(!xmlStrcmp(curNode->name, (const xmlChar*)"author"))
                {
                    for(xmlNodePtr authorNode = curNode->xmlChildrenNode; authorNode; authorNode = authorNode->next)
                    {
                        if(!xmlStrcmp(authorNode->name, (const xmlChar*)"name"))
                        {
                            content = xmlNodeGetContent(authorNode);
                            author.name = (char*)content;
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(authorNode->name, (const xmlChar*)"email"))
                        {
                            content = xmlNodeGetContent(authorNode);
                            author.email = (char*)content;
                            xmlFree(content);
                        }
                    }
                }

                if(!xmlStrcmp(curNode->name, (const xmlChar*)"entry"))
                {
                    for(xmlNodePtr childNode = curNode->xmlChildrenNode; childNode; childNode = childNode->next)
                    {
                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"title"))
                        {
                            content = xmlNodeGetContent(childNode);
                            write.title = (char*)content;
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"updated"))
                        {
                            content = xmlNodeGetContent(childNode);
                            write.time = (char*)content;
                            xmlFree(content);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"author"))
                        {
                            for(xmlNodePtr authorNode = childNode->xmlChildrenNode; authorNode; authorNode = authorNode->next)
                            {
                                if(!xmlStrcmp(authorNode->name, (const xmlChar*)"name"))
                                {
                                    content = xmlNodeGetContent(authorNode);
                                    author.name = (char*)content;
                                    xmlFree(content);
                                }

                                if(!xmlStrcmp(authorNode->name, (const xmlChar*)"email"))
                                {
                                    content = xmlNodeGetContent(authorNode);
                                    author.email = (char*)content;
                                    xmlFree(content);
                                }
                            }

                            write.authors.push_back(author);
                        }

                        if(!xmlStrcmp(childNode->name, (const xmlChar*)"link"))
                        {
                            content = xmlGetProp(childNode,(const xmlChar*)"href");
                            write.aURL.push_back((char*)content);
                            xmlFree(content);
                        }

                        if(write.authors.empty() && !author.name.empty())
                        {
                            write.authors.push_back(author);
                        }
                    }

                    writeList.push_back(write);
                    write.aURL.clear();
                    write.authors.clear();
                    write.time.clear();
                    write.title.clear();
                }
            }

            xmlCleanupParser();
            return writeList;
        }
        
        /**
         * @brief Print parsed items on standard output
         * 
         * @param content List of parsed items
         * @param title Title of the feed
         * @param args Class with command line arguments
         */
        void printInfo(std::vector<XMLContent> content, std::string title, Args *args)
        {
            cout << "*** " << title.c_str() << " ***" << endl;

            for(XMLContent cont : content)
            {
                if(cont.title.empty())
                {
                    cout << "<No Title>" << endl;
                }
                else cout << cont.title.c_str() << endl;

                if(args->isTime() && !cont.time.empty())
                {
                    cout << "Aktualizace: " << cont.time.c_str() << endl;
                }

                if(args->isAuthor() && !cont.authors.empty())
                {
                    for(Author author : cont.authors)
                    {
                        if(!author.email.empty())
                        {
                            cout << "Autor: " << author.name.c_str() << " (" << author.email.c_str() << ")" << endl;
                        }
                        else cout << "Autor: " << author.name.c_str() << endl;
                    }
                }

                if(args->isAssocURL() && !cont.aURL.empty())
                {
                    for(std::string url : cont.aURL)
                    {
                        cout << "URL: " << url.c_str() << endl;
                    }
                }

                cout << endl;
            }

            return;
        }
};