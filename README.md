# ngx-http-merics-module
This is a project of practical classes by Avalon course 'Unix system programming, 2016'

#Build: 
$ngx_path$/auto/configure --add-module=$absolute_path$/ngx-http-merics-module

#Usage:
In nginx.conf:
server {
    metrics_enable;
    
    location /metrics {
        metrics_status;
    }
}
