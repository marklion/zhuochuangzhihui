#!/bin/bash

ZH_DELIVER="zh_deliver.tar.gz"
WECHAT_SECRET_INPUT="none"
WECHAT_MP_SECRET_INPUT="none"
MAIL_PWD_INPUT="none"
PORT=""
DATA_BASE="zh.db"
IMG_BED_INPUT="logo_res"
DEVICE_CONFIG_FILE_INPUT="./device_config.json"
ALI_KEY_ID_INPUT="none"
ALI_KEY_SEC_INPUT="none"
DOCKER_IMG_NAME="zh_deploy:v1.0"
SRC_DIR=`dirname $(realpath $0)`/../
BASE_URL_INPUT=""
OEM_NAME_INPUT="卓创智汇"
OEM_SHORT_INPUT="卓创智汇"
URL_REMOTE_INPUT=".d8sis.cn"

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
    line=`expr $line - 139`
    mkdir /tmp/sys_zh
    tail -n $line $0 | tar zx  -C /tmp/sys_zh/
    rsync -aK /tmp/sys_zh/ /
    /conf/change_base_url.sh ${BASE_URL} /conf/frpc.ini
    [ "${BASE_URL}" != "" ] && frpc -c /conf/frpc.ini &
    cp /conf/ngx_http_flv_live_module.so /lib/nginx/modules/
    nginx -c /conf/nginx.conf
    service cups start
    wetty &
    ulimit -c unlimited
    sysctl -w kernel.core_pattern=/database/core.%e.%p.%s.%E
    ulimit -c
    ulimit -q 819200000
    device_center &
    pushd /api
    pm2 start index.js
    popd
    core_service
}

start_docker_con() {
    local DATA_BASE_PATH=`realpath $DATA_BASE`
    local DATA_BASE_PATH=`dirname ${DATA_BASE_PATH}`
    local DEVICE_CONFIG_FILE_PATH=`realpath ${DEVICE_CONFIG_FILE_INPUT}`
    local DEVICE_CONFIG_FILE_PATH=`dirname ${DEVICE_CONFIG_FILE_PATH}`
    local IMG_BED=`realpath $IMG_BED_INPUT`
    local MOUNT_PROC_ARG=''
    if [ -d /proc ]
    then
        MOUNT_PROC_ARG='-v /proc:/host/proc'
    fi
    local PORT_ARG="-p ${PORT}:80"
    if [ "$PORT" == "" ]
    then
        PORT_ARG=""
    fi
    local CON_ID=`docker create --privileged ${MOUNT_PROC_ARG} --restart=always ${PORT_ARG} -e BASE_URL=${BASE_URL_INPUT} -e OEM_NAME=${OEM_NAME_INPUT} -e WECHAT_SECRET="${WECHAT_SECRET_INPUT}" -e WECHAT_MP_SECRET="${WECHAT_MP_SECRET_INPUT}" -e ALI_KEY_ID="${ALI_KEY_ID_INPUT}" -e ALI_KEY_SEC="${ALI_KEY_SEC_INPUT}" -e MAIL_PWD="${MAIL_PWD_INPUT}" -e OEM_SHORT="${OEM_SHORT_INPUT}" -e URL_REMOTE="${URL_REMOTE_INPUT}" -v ${DATA_BASE_PATH}:/database -v ${DEVICE_CONFIG_FILE_PATH}:/conf/device -v ${IMG_BED}:/manage_dist/logo_res  ${DOCKER_IMG_NAME} /root/install.sh`
    docker cp $0 ${CON_ID}:/root/
    docker start ${CON_ID}
}

while getopts "D:p:w:d:i:m:a:k:M:g:b:o:O:u:" arg
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
        M)
            MAIL_PWD_INPUT=${OPTARG}
            ;;
        g)
            DEVICE_CONFIG_FILE_INPUT=${OPTARG}
            ;;
        b)
            BASE_URL_INPUT=${OPTARG}
            ;;
        o)
            OEM_NAME_INPUT=${OPTARG}
            ;;
        O)
            OEM_SHORT_INPUT=${OPTARG}
            ;;
        u)
            URL_REMOTE_INPUT=${OPTARG}
            ;;
        a)
            ALI_KEY_ID_INPUT=${OPTARG}
            ;;
        k)
            ALI_KEY_SEC_INPUT=${OPTARG}
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
