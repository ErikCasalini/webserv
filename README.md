*This project has been created as part of the 42 curriculum by ecasalin and juportie.*

# webserv

## Description

The 42 project's webserv is an http server loosely inspired by nginx (mostly for the config file). It roughly follows the rfc 1945 for HTTP/1.0 and the (pseudo) rfc 3875 for CGI Version 1.1.

### The goals of this project:

#### Understanding the HTTP protocol

- messages formating:
    - request:
        ```http
        GET / HTTP/1.0
        ```
    - response
        ```http
        HTTP/1.0 200 OK
        Content-Type: text/html
        Content-length: 41

        <html>
        <body>
            Hello!
        </body>
        </html>
        ```

- its stateless nature and how to simulate persistence with cookies:
    ```http
    Set-Cookie: key_1=value_1;...;key_n=value_n`
    ```

#### Understanding Cgi protocol and how it permits to execute server side scripts

> It's not used a lot anymore as it have a lot of potential security vulnerabilities and the need to launch a new process for each requests is time consuming. As exemple alternatives nginx implemented fast-cgi to avoid launching a process everytime and web frameworks could replace CGI entirely.

#### use OOP with C++ to develop the server

- config file parsing (preprocessing -> lexing -> parsing)
- linux sockets (managed with epoll)
- mono-threaded non blocking behavior (using epoll, implemented with state machines)
- requests and cgi responses parsing
- responses parsing and crafting
- cgi scripts handling (with processes, artificialy crafted environement and pipes)
- cgi responses parsing
- cookies (with an internal cookie jar to track cookies expiration)

## Instructions

### Build

- `make` build the executable
- `make test` build and execute the tests

### Usage

- launch the server with `webserv CONFIG_FILE`

### Config file

- structured as blocks and key value(s) fields
- each subblock inherit values from top blocks if not set
- allow `include` directive to include content from another file as `include FILE`
- allow comments
- value types:
    - bool: true/false
    - str
    - num

```conf
# a comment line
events { # an endline comment
    max_connections num;
}

http {
    autoindex           bool;
    *(error_page        *num str;)
    keepalive_timeout   num;
    max_body_size       num;
    root                str;
    default_type        str;
    types               {*(str *str;)} -> type extension(s);

    server {
        autoindex       bool;
        *(error_page    *num str;)
        *(listen        num:num;)      -> ip:port;
        root            str;

        *(locations {});
        location [=] str {              -> optional '=' is exact match, str is path
            autoindex       bool;
            cgi             bool;       -> set this path as cgi
            cgi_nph         bool;       -> set cgi in 'non parsing headers' mode 
            *(error_page    *num str;)
            index           str;
            limit_except    *str;       -> methods to allow, if not set all methods accepted
            redirection     num string; -> status uri
            root            str;
        }

        redirection num string; -> status uri
        root        str;
        storage     str;        -> path of the storage location
    }
}
```

## Ressources

- Rfcs
    - [Hypertext Transfer Protocol -- HTTP/1.0](https://www.rfc-editor.org/rfc/rfc1945.html)
    - [The Common Gateway Interface (CGI) Version 1.1](https://www.rfc-editor.org/rfc/rfc3875.html)

- Wikipedia
    - [HTTP](https://en.wikipedia.org/wiki/HTTP)
    - [Common_Gateway_Interface](https://en.wikipedia.org/wiki/Common_Gateway_Interface)

- Mozilla
    - [Overview of HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Overview)
    - [HTTP messages](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages)
    - [Using HTTP cookies](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Cookies)
    - [Media types (MIME types)](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types)
    - [HTTP reference](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference)

- IBM
    - [Using poll() instead of select()](https://www.ibm.com/docs/en/i/7.1.0?topic=designs-using-poll-instead-select)

### Inspiration

- [nginx](https://nginx.org/)
- [nginx config](https://nginx.org/en/docs/ngx_core_module.html)

### Llm

- Claude, chatGPT and ollama were used to:
    - clarify some concepts
    - give advices and opinions about OOP implementations choices and config parsing strategies
    - generate templates of html code
