/**
 *   \file ngx_http_metrics_module.c
 *   \brief Nginx metrics module 
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
  ngx_uint_t enable;
} ngx_http_metrics_main_conf_t;

static ngx_int_t
ngx_http_metrics_init(ngx_conf_t *cf);
static ngx_int_t
ngx_http_metrics_process_init(ngx_cycle_t *cycle);

static void *
ngx_http_metrics_create_main_conf(ngx_conf_t *cf);

static char *
ngx_http_metrics_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

ngx_int_t
ngx_http_metrics_begin_handler(ngx_http_request_t *r);
ngx_int_t
ngx_http_metrics_handler(ngx_http_request_t *r);

static ngx_command_t
ngx_http_metrics_commands[] = {

  { ngx_string("metrics_enable"),
    NGX_HTTP_MAIN_CONF | NGX_CONF_ANY,
    ngx_http_metrics_enable,
    NGX_HTTP_MAIN_CONF_OFFSET,
    0,
    NULL },
  
  ngx_null_command
};

static ngx_http_module_t
ngx_http_metrics_module_ctx = {
  NULL,                                   /* preconfiguration */
  ngx_http_metrics_init,                  /* postconfiguration */
  
  ngx_http_metrics_create_main_conf,      /* create main configuration */
  NULL,                                   /* init main configuration */

  NULL,                                   /* create server configuration */
  NULL,                                   /* merge server configuration */

  NULL,                                   /* create location configuration */
  NULL                                    /* merge location configuration */
};

ngx_module_t
ngx_http_metrics_module = {
  NGX_MODULE_V1,
  &ngx_http_metrics_module_ctx,           /* module context */
  ngx_http_metrics_commands,              /* module commands */
  NGX_HTTP_MODULE,                        /* module type */
  NULL,                                   /* init master */
  NULL,                                   /* init module */
  ngx_http_metrics_process_init,          /* init process */
  NULL,                                   /* init thread */
  NULL,                                   /* exit thread */
  NULL,                                   /* exit process */
  NULL,                                   /* exit master */
  NGX_MODULE_V1_PADDING
};
  
static ngx_int_t
ngx_http_metrics_init(ngx_conf_t *cf)
{
  ngx_http_handler_pt       *h;
  ngx_http_core_main_conf_t *cmcf;

  fprintf(stderr, "METRICS: call ngx_http_metrics_init\n");

  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

  h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);

  if (h == NULL) {
    fprintf(stderr, "METRICS: Can't create end handler\n");
    return NGX_ERROR;
  }

  *h = ngx_http_metrics_handler;
  
  return NGX_OK;
}
  
static ngx_int_t
ngx_http_metrics_process_init(ngx_cycle_t *cycle)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_process_init\n");
  return 0;
}

static void *
ngx_http_metrics_create_main_conf(ngx_conf_t *cf)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_create_main_conf\n");

  ngx_http_metrics_main_conf_t *mmcf;
  mmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_metrics_main_conf_t));

  if (mmcf == NULL) {
    return NULL;
  }

  return mmcf;
}

static char *
ngx_http_metrics_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_config\n");
  return 0;
}

ngx_int_t
ngx_http_metrics_handler(ngx_http_request_t *r)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_end_handler\n");

  ngx_http_metrics_main_conf_t *mmcf;

  mmcf = ngx_http_get_module_main_conf(r, ngx_http_metrics_module);

  if (!mmcf->enable) {
    fprintf(stderr, "METRICS: metrics is disabled\n");
    return NGX_OK;
  }

  time_t ts = ngx_time();

  fprintf(stderr, "METRICS: Time end = %ld\n", ts);
  
  return NGX_OK;
}
