Generative API Art
---

#### Installation

Copy `config.hh.dist` to `config.hh` and change your API key.

#### Example nginx configuration

```nginx
server {

    server_name example.com;

    index index.html index.php index.hh;
    root /www/generativeapiart/;

    error_log /var/log/nginx/generativeapiart.error.log;

    # Searching aliases
    rewrite ^/(.*) /index.hh?q=$1 last;

    location = / {
          try_files $uri $uri/ /index.hh?$query_string;
    }

    # Pass on to HHVM
    include hhvm.conf;

    # Favicons and robots
    include favicon.robots.conf;

    # deny access to .htaccess files
    location ~ /\.ht {
      deny all;
    }
}
```