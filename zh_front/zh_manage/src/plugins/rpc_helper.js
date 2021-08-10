import { Loading } from 'element-ui';
import { Message } from 'element-ui';
export function get_client(_service_interface) {
        // eslint-disable-next-line
    var mp1 = new Thrift.Multiplexer();
        // eslint-disable-next-line
    var transport = new Thrift.Transport("http://localhost:8123/");
        // eslint-disable-next-line
    var protocol = new Thrift.Protocol(transport);
        // eslint-disable-next-line
    var client = mp1.createClient(_service_interface, eval(_service_interface + 'Client'), transport);
    return client;
}

export function call_remote_process(_service_interface,process,args) {
    var client = get_client(_service_interface);
    const toast1 = Loading.service({
        lock: true,
        text: 'Loading',
    });
    return new Promise(function (resolve, reject) {
        client[process].apply(client, args).then(function (resp) {
            resolve(resp);
        }).catch(function (err) {
            Message.error({message: err.msg});
            console.log(err);
            reject(err);
        }).finally(function () {
            toast1.close();
        });
    });

}
export function call_remote_process_no_toast(_service_interface,process,args) {
    var client = get_client(_service_interface);
    return new Promise(function (resolve, reject) {
        client[process].apply(client, args).then(function (resp) {
            resolve(resp);
        }).catch(function (err) {
            console.log(err);
            reject(err);
        });
    });

}