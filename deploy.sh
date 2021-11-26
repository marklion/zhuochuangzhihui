#!/bin/bash

ZH_DELIVER="zh_deliver.tar.gz"
WECHAT_SECRET_INPUT="none"
WECHAT_MP_SECRET_INPUT="none"
HK_KEY_INPUT="none"
HK_SEC_INPUT="none"
MAIL_PWD_INPUT="none"
PORT=80
DATA_BASE="zh.db"
IMG_BED_INPUT="logo_res"
DEVICE_CONFIG_FILE_INPUT="./device_config.json"

DOCKER_IMG_NAME="zh_deploy:v1.0"
SRC_DIR=`dirname $(realpath $0)`/../
is_in_container() {
    cat /proc/1/cgroup | grep pids | grep docker 2>&1>/dev/null
}

make_docker_img_from_dockerfile() {
    docker build -t ${DOCKER_IMG_NAME} ${SRC_DIR}/.devcontainer/
}

get_docker_image() {
    docker images ${DOCKER_IMG_NAME} | grep zh_deploy > /dev/null
    if [ $? != 0 ]
    then
        make_docker_img_from_dockerfile
    fi
}

start_all_server() {
    line=`wc -l $0|awk '{print $1}'`
    line=`expr $line - 103`
    tail -n $line $0 | tar zx  --skip-old-files -C /
    frpc -c /conf/frpc.ini &
    nginx -c /conf/nginx.conf
    zh_daemon &
    pushd /zh_rest_node
    pm2 start index.js
    popd
    bash
}

start_docker_con() {
    local DATA_BASE_PATH=`realpath $DATA_BASE`
    local DATA_BASE_PATH=`dirname ${DATA_BASE_PATH}`
    local DEVICE_CONFIG_FILE_PATH=`realpath ${DEVICE_CONFIG_FILE_INPUT}`
    local DEVICE_CONFIG_FILE_PATH=`dirname ${DEVICE_CONFIG_FILE_PATH}`
    local IMG_BED=`realpath $IMG_BED_INPUT`
    local CON_ID=`docker create --privileged -ti --rm -p ${PORT}:80 -e WECHAT_SECRET="${WECHAT_SECRET_INPUT}" -e WECHAT_MP_SECRET="${WECHAT_MP_SECRET_INPUT}" -e HK_KEY="${HK_KEY_INPUT}" -e HK_SEC="${HK_SEC_INPUT}" -e MAIL_PWD="${MAIL_PWD_INPUT}" -v ${DATA_BASE_PATH}:/database -v ${DEVICE_CONFIG_FILE_PATH}:/conf/device -v ${IMG_BED}:/manage_dist/logo_res  ${DOCKER_IMG_NAME} /root/install.sh`
    docker cp $0 ${CON_ID}:/root/
    docker start -ai ${CON_ID}
}

while getopts "D:p:w:d:i:m:s:k:M:g:" arg
do
    case $arg in
        D)
            PA_DELIVER=${OPTARG}
            ;;
        p)
            PORT=${OPTARG}
            ;;
        w)
            WECHAT_SECRET_INPUT=${OPTARG}
            ;;
        m)
            WECHAT_MP_SECRET_INPUT=${OPTARG}
            ;;
        d)
            DATA_BASE=${OPTARG}
            ;;
        i)
            IMG_BED_INPUT=${OPTARG}
            ;;
        k)
            HK_KEY_INPUT=${OPTARG}
            ;;
        s)
            HK_SEC_INPUT=${OPTARG}
            ;;
        M)
            MAIL_PWD_INPUT=${OPTARG}
            ;;
        g)
            DEVICE_CONFIG_FILE_INPUT=${OPTARG}
            ;;
        *)
            echo "invalid args"
            ;;
    esac
done

if is_in_container
then
    start_all_server
else
    get_docker_image
    start_docker_con
fi

#
exit
