# PROJECT
ISA Projekt - Newsfeed reader in Atom or RSS 2.0 format with TLS support
## Description
Feedreader will write out information from downloaded feeds in Atom or RSS 2.0 format
## Usage
### Compilation
Command
```
make
```
will create executable script
### Executing program
```
./feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T][-u][-a][-h | --help]
```
* Required parameter -> URL 		> URL address to be read ``OR``
* Required parameter -> -f <feedfile> > path to file with URLs to be read
* Optional parameter -> -c <certfile> > path to file with certificates to be used
* Optional parameter -> -C <certaddr> > path to directory with certificates to be used
* Optional parameter -> -T		> script will write out time of last modification
* Optional parameter -> -u		> script will write out associated URLs
* Optional parameter -> -a		> script will write out information about author of feed
* Optional parameter -> -h | --help	> script will write out ``Help statement``

### Using program
* User have to use one of required parameters, but NOT both.
* User can specify his own port in URL.
* If does user use TLS, connection will happen only if it is trustworthy
* Script does not have implemented redirecting in HTTP communication
* If one of the links in feedfile fail, script will continue with the next link
### Removing
Command
```
make clean
```
will remove all executable and .o files

### Testing
Command
```
make test
```
will start test script `test.sh` which will test script `feedreader`.
For more details use command
```
make test_help
```

## Author
Denis Horil (xhoril01)
## Sources
[Atom feed](https://validator.w3.org/feed/docs/atom.html#requiredFeedElements)
[RSS 2.0 specification](https://www.rssboard.org/rss-specification#google_vignette)
[RDF specification](https://validator.w3.org/feed/docs/rss1.html#s7)
[RFC 4287 - atom format](https://www.rfc-editor.org/rfc/rfc4287)
[OpenSSL tutorial](https://developer.ibm.com/tutorials/l-openssl/)
[Libxml examples](https://github.com/yarox/libxml-examples/tree/master/src/c)
