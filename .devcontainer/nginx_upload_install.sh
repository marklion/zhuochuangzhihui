#!/bin/bash

NGINX_VERSION=`nginx -v 2>&1 | awk -F '/' '{print $NF}' | awk '{print $1}'`

wget http://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz
tar xf nginx-${NGINX_VERSION}.tar.gz
git clone https://github.com/vkholodkov/nginx-upload-module.git

NGINX_CONF_ARG=`nginx -V 2>&1 | grep configure | awk -F 'arguments: ' '{print $NF}'`

pushd nginx-${NGINX_VERSION}

echo "./configure ${NGINX_CONF_ARG} --add-dynamic-module=../nginx-upload-module" | bash
make modules 
cp objs/ngx_http_upload_module.so /lib/nginx/modules/
popd